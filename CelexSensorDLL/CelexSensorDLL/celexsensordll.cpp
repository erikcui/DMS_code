
#include <strstream>
#include <iostream>
#include <cstring>
#include "celexsensordll.h"
#include "frontpanel.h"
#include "hhsequencemgr.h"
#include "xbase.h"
#include "dataprocessthread.h"
#include "fpgadatareader.h"
#include "hhconstants.h"

void SensorData::registerData(Observer* observer, Observer::emDataType type)
{
    m_pObserver = observer;
    m_listDataType.push_back(type);
}

void SensorData::unregisterData(Observer* observer, Observer::emDataType type)
{
    if (observer == m_pObserver)
    {
        m_listDataType.remove(type);
    }
}

void SensorData::notify(Observer::emDataType dataType)
{
    if (m_pObserver)
    {
        m_pObserver->updateData(dataType);
    }
}

// This is the constructor of a class that has been exported.
// see CelexSensorDLL.h for the class definition
CelexSensorDLL::CelexSensorDLL()
    : m_uiTimeSlice(30)
	, m_uiOverlapTime(0)
	, m_uiClockRate(25)
    , m_iTimeStartRecord(0)
    , m_bRecording(false)
    , m_lPlaybackFileSize(0)
{
    m_pData = new unsigned char[BUFFER_SIZE];

    m_pFrontPanel = FrontPanel::getInstance();

    m_pSequenceMgr = new HHSequenceMgr;
    m_pSequenceMgr->parseCommandList();
    m_pSequenceMgr->parseSequenceList();
    m_pSequenceMgr->parseSliderList();

    parseSliderSequence();

    //create data process thread
    m_pDataProcessThread = new DataProcessThread;
    m_pDataProcessThread->start();
}

CelexSensorDLL::~CelexSensorDLL()
{
    if (m_ifstreamPlayback.is_open())
        m_ifstreamPlayback.close();

    if (m_pSequenceMgr)
    {
        delete m_pSequenceMgr;
    }
	if (m_pDataProcessThread)
	{
		m_pDataProcessThread->terminate();
		delete m_pDataProcessThread;
	}
    if (m_pFrontPanel)
    {
        m_pFrontPanel->uninitializeFPGA();
        delete m_pFrontPanel;
    }
    if (m_pData)
    {
        delete[] m_pData;
    }
}

CelexSensorDLL::ErrorCode CelexSensorDLL::openSensor(string str)
{
    initializeFPGA();
    if (isSensorReady())
    {
        if (!powerUp())
            return CelexSensorDLL::PowerUpFailed;
        if (!configureSettings())
            return CelexSensorDLL::ConfigureFailed;

        //cout << "****************************************" << endl;
		//excuteCommand("SetMode Total Time");
    }
    else
    {
        return CelexSensorDLL::InitializeFPGAFailed;
    }
    return CelexSensorDLL::NoError;
}

void CelexSensorDLL::initializeFPGA(uint32_t value)
{
    if (0 == value)
    {
        m_pFrontPanel->initializeFPGA();
    }
    else
    {
        setThreshold(100);
        std::strstream ss;
        ss << "top" << value << ".bit";
        std::string bitfileName = ss.str();
        m_pFrontPanel->initializeFPGA(bitfileName);
    }
}

// Execute Sensor Power-up Sequence
bool CelexSensorDLL::powerUp()
{
    bool bSuccess = false;
    std::string sPowerUp = "Power Up";
    if (HHSequence* pUpSeq = m_pSequenceMgr->getSequenceByName(sPowerUp))
    {
        if (!pUpSeq->fire())
            cout << "Power Up failed." << endl;
        else
            bSuccess = true;
    }
    else
    {
        cout << sPowerUp << ": Sequence not defined." << endl;
    }
    return bSuccess;
}

bool CelexSensorDLL::configureSettings()
{
    bool bOk = false;
    std::string settingNames;
    if (m_pFrontPanel->isReady())
    {
        bOk = true;
        int index = 0;
        for (vector<string>::iterator itr = m_vecAdvancedNames.begin(); itr != m_vecAdvancedNames.end(); itr++)
        {
            bool ret = setAdvancedBias(emAdvancedBiasType(index));
            if (!ret)
                settingNames += (" " + (*itr));
            bOk = (bOk && ret);
            ++index;
        }
    }
    if (bOk)
        cout << "Configure Advanced Settings Successfully!" << endl;
    else
        cout << "Configure Advanced Settings Failed @" << settingNames << endl;
    return bOk;
}

bool CelexSensorDLL::isSensorReady()
{
    return m_pFrontPanel->isReady();
}

bool CelexSensorDLL::isSdramFull()
{
    uint32_t sdramFull;
    FrontPanel::getInstance()->wireOut(0x20, 0x0001, &sdramFull);
    if (sdramFull > 0)
    {
        cout << "---- SDRAM is full! -----" << endl;
        return true;
    }
    return false;
}

void CelexSensorDLL::pipeOutFPGAData()
{
	if (!isSensorReady())
	{
		return;
	}
	uint32_t pageCount;
	m_pFrontPanel->wireOut(0x21, 0x1FFFFF, &pageCount);
	m_uiPageCount = pageCount;
	//cout << "----------- pageCount = " << pageCount << endl;
	int blockSize = 128;
	long length = 0;
	long dataLen = 0;
	if (pageCount >= 10)
	{
		if (pageCount > 100000)
		{
			pageCount = 100000;
		}
		length = (long)(pageCount * blockSize);

		//Return the number of bytes read or ErrorCode (<0) if the read failed. 
		dataLen = m_pFrontPanel->blockPipeOut(0xa0, blockSize, length, m_pData);
	}
	if (dataLen > 0)
	{
		//record sensor data
		if (m_bRecording)
		{
			m_ofstreamRecord.write((char*)m_pData, dataLen);
		}
		unsigned char* dataIn = new unsigned char[dataLen];
		memcpy(dataIn, m_pData, dataLen);
		m_pDataProcessThread->addData(dataIn, dataLen);
	}
	else if (dataLen < 0) //read failed
	{
		switch (dataLen)
		{
		case okCFrontPanel::InvalidBlockSize:
			cout << "Block Size Not Supported" << endl;
			break;

		case okCFrontPanel::UnsupportedFeature:
			cout << "Unsupported Feature" << endl;
			break;

		default:
			cout << "Transfer Failed with error: " << dataLen << endl;
			break;
		}
		cout << "pageCount = " << pageCount << ", blockSize = " << blockSize << ", length = " << length << endl;
	}
}

unsigned char *CelexSensorDLL::getFullPicBuffer()
{
    return g_pBufferImage0;
}

unsigned char *CelexSensorDLL::getEventPicBuffer(emEventPicMode mode)
{
	if (EventAccumulationPic == mode)
	{
		return g_pBufferImage0;
	}
    else if (EventOnlyPic == mode)
    {
        return g_pBufferImage1;
    }
	else if (EventGrayPic == mode)
	{
		return g_pBufferImage2;
	}
	else if (EventSuperimposedPic == mode)
	{
		return g_pBufferImage3;
	}
	else if (EventDenoisedBinaryPic == mode)
	{
		return g_pBufferImage4;
	}
	else if (EventDenoisedGrayPic == mode)
	{
		return g_pBufferImage5;
	}
}

void CelexSensorDLL::getPixelData(int &row, int &column, int &brightness, int &time)
{
	SensorData* pSensorData = getSensorDataObject();
	if (pSensorData)
	{
		row = pSensorData->getRow();
		column = pSensorData->getColumn();
		brightness = pSensorData->getBrightness();
		time = pSensorData->getTime();
	}
}

void CelexSensorDLL::clearData()
{
    m_pDataProcessThread->clearData();
}

// Execute Sensor "Event Mode"/"Full Picture" Sequence
void CelexSensorDLL::setSensorMode(emSensorMode mode)
{
    if (!m_pFrontPanel->isReady())
        return;

    if (mode == EventMode)
    {
        excuteCommand("Full Picture");
        m_pDataProcessThread->setSensorMode(mode);
    }
    else if (mode == FullPictureMode)
    {
        excuteCommand("Event Mode");
        m_pDataProcessThread->setSensorMode(mode);
    }
    else //AC or AB or ABC Mode
    {
        excuteCommand("Optical Mode");
        m_pDataProcessThread->setSensorMode(mode);
    }
}

emSensorMode CelexSensorDLL::getSensorMode()
{
    return m_pDataProcessThread->getDataProcessor()->getSensorMode();
}

bool CelexSensorDLL::setFpnFile(const string &fpnFile)
{
    return m_pDataProcessThread->getDataProcessor()->setFpnFile(fpnFile);
}

void CelexSensorDLL::generateFPN(std::string fpnFile)
{
    m_pDataProcessThread->getDataProcessor()->generateFPN(fpnFile);
}

void CelexSensorDLL::resetDevice(emDeviceType emType)
{
    if (emType == FPGA)
    {
        excuteCommand("Reset-Dereset FPGA");
    }
    else if (emType == SensorAndFPGA)
    {
        excuteCommand("Reset-Dereset All");
    }
    else
    {
        cout << "Can't reset this type: " << (int)emType << endl;
    }
}

void CelexSensorDLL::enableADC(bool enable)
{
    if (enable)
        excuteCommand("ADC Disable");
    else
        excuteCommand("ADC Enalbe");
}

void CelexSensorDLL::setContrast(uint32_t value)
{
    uint32_t uiREF_PLUS = 512 + value / 2;
    uint32_t uiREF_MINUS = 512 - value / 2;
    uint32_t uiREF_PLUS_H = REF_PLUS + value / 16;
    uint32_t uiREF_MINUS_H = REF_MINUS + value / 16;

    setAdvancedBias(CelexSensorDLL::REF_PLUS);
    setAdvancedBias(CelexSensorDLL::REF_MINUS);
    setAdvancedBias(CelexSensorDLL::REF_PLUS_H);
    setAdvancedBias(CelexSensorDLL::REF_MINUS_H);

    m_mapSliderNameValue["REF+"] = uiREF_PLUS;
    m_mapSliderNameValue["REF-"] = uiREF_MINUS;
    m_mapSliderNameValue["REF+H"] = uiREF_PLUS_H;
    m_mapSliderNameValue["REF-H"] = uiREF_MINUS_H;

    m_uiContrast = value;
}

uint32_t CelexSensorDLL::getContrast()
{
    return m_uiContrast;
}

void CelexSensorDLL::setBrightness(uint32_t value)
{
    setAdvancedBias(CelexSensorDLL::CDS_DC);
    m_mapSliderNameValue["CDS_DC"] = value;

    m_uiBrightness = value;
}

uint32_t CelexSensorDLL::getBrightness()
{
    return m_uiBrightness;
}

void CelexSensorDLL::setThreshold(uint32_t value)
{
    unsigned uiEVT_DC = m_mapSliderNameValue["EVT_DC"];
    uint32_t uiEVT_VL = uiEVT_DC - value;
    uint32_t uiEVT_VH = uiEVT_DC + value;

    setAdvancedBias(CelexSensorDLL::EVT_VL, uiEVT_VL);
    setAdvancedBias(CelexSensorDLL::EVT_VH, uiEVT_VH);

    m_mapSliderNameValue["EVT_VL"] = uiEVT_VL;
    m_mapSliderNameValue["EVT_VH"] = uiEVT_VH;

    m_uiThreshold = value;
}

uint32_t CelexSensorDLL::getThreshold()
{
    return m_uiThreshold;
}

void CelexSensorDLL::trigFullPic()
{
    excuteCommand("Force Fire");
}

void CelexSensorDLL::setLowerADC(uint32_t value)
{
    m_pDataProcessThread->getDataProcessor()->setLowerADC(value);
}

void CelexSensorDLL::setUpperADC(uint32_t value)
{
    m_pDataProcessThread->getDataProcessor()->setUpperADC(value);
}

void CelexSensorDLL::startRecording(std::string filePath)
{
    XBase base;
    m_iTimeStartRecord = base.getTimeStamp();

    m_ofstreamRecord.open(filePath.c_str(), std::ios::binary);
    if (!m_ofstreamRecord.is_open())
        cout << "Can't open recording file." << endl;

    // write a header
//    char header[8];
//    int value = 27000;
//    header[0] =      value  & 0x000000FF;
//    header[1] =  (value>>8) & 0x000000FF;
//    header[2] = (value>>16) & 0x000000FF;
//    header[3] = (value>>24) & 0x000000FF;
//    header[4] = 0;
//    header[5] = 0;
//    header[6] = 0;
//    header[7] = m_pDataProcessThread->getSensorMode(); // !
//    m_ofstreamRecord.write(header, sizeof(header));

    m_bRecording = true;
}

void CelexSensorDLL::stopRecording()
{
    XBase base;

    int itimeRecorded = base.getTimeStamp() - m_iTimeStartRecord;

    int hour   = itimeRecorded / 3600;
    int minute = (itimeRecorded % 3600) / 60;
    int second = (itimeRecorded % 3600) % 60;

    cout << itimeRecorded << endl;

    // write a header
    char header[8];
    header[0] = second;
    header[1] = minute;
    header[2] = hour;
    header[3] = 0;
    header[4] = 0;
    header[5] = 0;
    header[6] = m_uiClockRate;
    header[7] = m_pDataProcessThread->getDataProcessor()->getSensorMode(); // !

    m_ofstreamRecord.seekp(0, ios::beg);
    m_ofstreamRecord.write(header, sizeof(header));
    m_bRecording = false;
    m_ofstreamRecord.close();
}

CelexSensorDLL::TimeInfo CelexSensorDLL::getRecordedTime()
{
    return m_stTimeRecorded;
}

bool CelexSensorDLL::readPlayBackData(long length)
{
    //cout << __FUNCTION__ << endl;
    bool eof = false;
    int lenToRead = length;

    char* data = new char[lenToRead];
    int dataLen = 0;
    int seCount = 0;
    while (true && m_pDataProcessThread->queueSize() < 1000000/*000*/)
    {

        m_ifstreamPlayback.read((char*)data, lenToRead);

        if (!m_ifstreamPlayback.eof())
        {
            dataLen = lenToRead;
            unsigned char* dataIn = new unsigned char[dataLen];
            memcpy(dataIn, data, dataLen);
            m_pDataProcessThread->addData((unsigned char*)dataIn, dataLen);
            if (lenToRead == 4 && FPGADataReader::isSpecialEvent(dataIn))
            {
                if (++seCount > 1)
                    break;
            }
        }
        else
        {
            eof = true;
            //m_ifstreamPlayback.close();
            cout << "Read Playback file Finished!" << endl;
            break;
        }
    }
    delete[] data;
    return eof;
}

bool CelexSensorDLL::openPlaybackFile(string filePath)
{
    if (m_ifstreamPlayback.is_open())
        m_ifstreamPlayback.close();

    m_ifstreamPlayback.open(filePath.c_str(), std::ios::binary);
    if (!m_ifstreamPlayback.good())
    {
        cout << "Can't Open File: " << filePath.c_str();
        return false;
    }

    // read header
    char header[8];
    m_ifstreamPlayback.read(header, 8);
    if ((header[0] != 0 || header[1] != 0 || header[2] != 0) && header[3] == 123)
    {
        m_stTimeRecorded.second = header[0];
        m_stTimeRecorded.minute = header[1];
        m_stTimeRecorded.hour   = header[2];
    }
    else
    {
        m_ifstreamPlayback.seekg(0, ios::end);
        m_lPlaybackFileSize = m_ifstreamPlayback.tellg();
        long aa = 1000000;
        long time = (m_lPlaybackFileSize/aa) * 3;

        m_ifstreamPlayback.seekg(0, ios::beg);
        m_stTimeRecorded.second = (time%3600)%60;
        m_stTimeRecorded.minute = (time%3600)/60;
        m_stTimeRecorded.hour   = time/3600;
    }
    if (header[7] > 0)
        m_pDataProcessThread->setSensorMode((emSensorMode)header[7]);
    else
        m_pDataProcessThread->setSensorMode(EventMode);

	if (header[6] > 1)
	{
		m_pDataProcessThread->getDataProcessor()->setFPGATimeCycle(131072);
		m_uiClockRate = header[6];
		m_pDataProcessThread->getDataProcessor()->setClockRate(m_uiClockRate);
	}
	else
	{
		m_pDataProcessThread->getDataProcessor()->setFPGATimeCycle(262144);
		m_uiClockRate = 25;
		m_pDataProcessThread->getDataProcessor()->setClockRate(m_uiClockRate);
	}
    cout << "sensor mode = " << (int)header[7] << ", clock = " << (int)header[6] << endl;
    return true;
}

void CelexSensorDLL::play()
{
    //if (!m_pDataProcessThread->isRunning())
        m_pDataProcessThread->resume();
}

void CelexSensorDLL::pause()
{
    if (m_pDataProcessThread->isRunning())
        m_pDataProcessThread->suspend();
}

long CelexSensorDLL::getPlaybackFileSize()
{
    return m_lPlaybackFileSize;
}

bool CelexSensorDLL::setPlayBackOffset(long offset)
{
    m_pDataProcessThread->clearData();
    //if (m_ifstreamPlayback.eof())
    {
        m_ifstreamPlayback.clear();
    }
    m_ifstreamPlayback.seekg(offset, ios::beg);

    return true;
}

void CelexSensorDLL::saveSelectedBinFile(string filePath, long fromPos, long toPos, int hour, int minute, int second)
{
    if (fromPos == toPos)
        return;

    std::ofstream ofstream;
    ofstream.open(filePath.c_str(), std::ios::binary);
    if (!m_ofstreamRecord.is_open())
        cout << "Can't open saveSelectedBinFile." << endl;

    // write a header
    char header[8];
    header[0] = second;
    header[1] = minute;
    header[2] = hour;
    header[3] = 123;
    header[4] = 0;
    header[5] = 0;
    header[6] = m_uiClockRate;
    header[7] = m_pDataProcessThread->getDataProcessor()->getSensorMode(); // !

    ofstream.write(header, sizeof(header));

    long lenToRead = toPos - fromPos;
    char* data = new char[lenToRead];

    m_ifstreamPlayback.clear();
    m_ifstreamPlayback.seekg(fromPos, ios::beg);
    m_ifstreamPlayback.read((char*)data, lenToRead);
    ofstream.write(data, lenToRead);
    ofstream.close();
}

//In Event mode, time block is calculated by software, so it needn't be set the FGPA
//In FullPic and FullPic_Event mode, time block is calculated by FPGA, so it need be set the FGPA
//The usrs hope to set the time block to 1s in FE mode, but there aren't enough bits, 
//in order to solve this problem, FPGA multiplys the msec by 4. 
void CelexSensorDLL::setTimeBlock(uint32_t msec)
{
    m_uiTimeSlice = msec;
	if (EventMode == getSensorMode())
	{
		m_pDataProcessThread->getDataProcessor()->setTimeSlice(msec);
	}
	else
	{
		uint32_t value = 0;
		if (FullPictureMode == getSensorMode())
			value = m_uiTimeSlice * 25000;
		else
			value = m_uiTimeSlice * 6250; //6250 = 25000/4 
		//--- excuteCommand("SetMode Total Time"); ---
		FrontPanel::getInstance()->wireIn(0x02, value, 0x00FFFFFF);
		FrontPanel::getInstance()->wait(1);
		cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0x00FFFFFF << endl;
	}
}

uint32_t CelexSensorDLL::getTimeBlock()
{
    return m_uiTimeSlice;
}

void CelexSensorDLL::setOverlapTime(uint32_t msec)
{
	m_uiOverlapTime = msec;
	m_pDataProcessThread->getDataProcessor()->setOverlapTime(msec);
}

uint32_t CelexSensorDLL::getOverlapTime()
{
	return m_uiOverlapTime;
}

SensorData *CelexSensorDLL::getSensorDataObject()
{
    return m_pDataProcessThread->getDataProcessor()->getSensorDataObject();
}

std::vector<CelexSensorDLL::ControlSliderInfo> CelexSensorDLL::getSensorControlList()
{
    return m_vecSensorControlList;
}

void CelexSensorDLL::pauseThread(bool pause1)
{
    if (pause1)
        pause();
    else
        play();
}

// init sliders
void CelexSensorDLL::parseSliderSequence()
{
    cout << endl << "***** " << __FUNCTION__ << " *****" << endl;
    std::vector<std::string> sliderNames = m_pSequenceMgr->getAllSliderNames();
    for (vector<string>::iterator itr = sliderNames.begin(); itr != sliderNames.end(); itr++)
    {
        std::string name = *itr;
        HHSequenceSlider* pSliderSeq = m_pSequenceMgr->getSliderByName(name);

        if (!pSliderSeq)
            continue;
        // show or not
        if (!pSliderSeq->isShown())
            continue;
        //save all the control class
        ControlSliderInfo sliderInfo;
        sliderInfo.min = pSliderSeq->getMin();
        sliderInfo.max = pSliderSeq->getMax();
        sliderInfo.value = pSliderSeq->getValue();
        sliderInfo.step = pSliderSeq->getStep();
        sliderInfo.name = pSliderSeq->name();
        sliderInfo.bAdvanced = pSliderSeq->isAdvanced();
        m_vecSensorControlList.push_back(sliderInfo);
        // advanced
        if (pSliderSeq->isAdvanced())
        {
            m_vecAdvancedNames.push_back(name);
            if (HHSequenceSlider* pSliderSeq = m_pSequenceMgr->getSliderByName(name))
            {
                uint32_t initial = pSliderSeq->getValue();
                // keep the initial values
                m_mapSliderNameValue[name] = initial;
                //cout << "m_mapSliderNameValue: " << name << "  " << initial << endl;
            }

            cout << "m_vecAdvancedNames: " << name << endl;
            continue;
        }
        uint32_t initial = pSliderSeq->getValue();
        // keep the initial values
        m_mapSliderNameValue[name] = initial;
        //cout << "m_mapSliderNameValue: " << name << "  " << initial << endl;
    }
}

bool CelexSensorDLL::excuteCommand(string strCommand)
{
    bool bSuccess = false;
    if (HHSequence* pUpSeq = m_pSequenceMgr->getSequenceByName(strCommand))
    {
        if (!pUpSeq->fire())
            cout << "excute command failed." << endl;
        else
            bSuccess = true;
    }
    else
    {
        cout << strCommand << ": Sequence not defined." << endl;
    }
    return bSuccess;
}

bool CelexSensorDLL::setAdvancedBias(CelexSensorDLL::emAdvancedBiasType biasType)
{
	bool result = false;
	std::string biasName = m_vecAdvancedNames[int(biasType)];
	if (HHSequenceSlider* pSeqSlider = m_pSequenceMgr->getSliderByName(biasName))
	{
		uint32_t arg = m_mapSliderNameValue[biasName];
		result = pSeqSlider->fireWithArg(arg);
		cout << "Advanced Settings loaded: " << biasName << "  " << arg << endl;
	}
	return result;
}

bool CelexSensorDLL::setAdvancedBias(CelexSensorDLL::emAdvancedBiasType biasType, int value)
{
    bool result = false;
    std::string biasName = m_vecAdvancedNames[int(biasType)];
    if (HHSequenceSlider* pSeqSlider = m_pSequenceMgr->getSliderByName(biasName))
    {
        result = pSeqSlider->fireWithArg(value);
        cout << "Advanced Settings loaded: " << biasName << "  " << value << endl;
    }
    return result;
}

uint32_t CelexSensorDLL::getClockRate()
{
	return m_uiClockRate;
}
//value * 2 = clock = 100 * M / D
void CelexSensorDLL::setClockRate(uint32_t value)
{
	cout << "************* API: setClockRate " << value << " MHz" << endl;
	m_uiClockRate = value;
	if (value > 50)
		value = 50;
	uint32_t valueM, valueD = 0x00630000;
	valueM = (value * 2 - 1) << 24 ;
	FrontPanel::getInstance()->wireIn(0x03, valueM, 0xFF000000); //M: [31:24]
	FrontPanel::getInstance()->wireIn(0x03, valueD, 0x00FF0000); //D: [23:16]

	FrontPanel::getInstance()->wireIn(0x03, 0, 0x00008000); //Apply OFF [15]
	FrontPanel::getInstance()->wait(1);
	FrontPanel::getInstance()->wireIn(0x03, 0x00008000, 0x00008000); //Apply ON  [15]

	m_pDataProcessThread->getDataProcessor()->setClockRate(value);
}

void CelexSensorDLL::enableMultiSlice(bool enable)
{
	m_pDataProcessThread->getDataProcessor()->enableMultiSlice(enable);
}

bool CelexSensorDLL::isMultiSliceEnabled()
{
	return m_pDataProcessThread->getDataProcessor()->isMultiSliceEnabled();
}

void CelexSensorDLL::setMultiSliceTime(uint32_t msec)
{
	cout << "************* API: setMultiSliceTime " << msec << " ms" << endl;
	m_pDataProcessThread->getDataProcessor()->setMultiSliceTime(msec);
}

void CelexSensorDLL::setMultiSliceCount(uint32_t count)
{
	cout << "************* API: setMultiSliceCount " << count << endl;
	m_pDataProcessThread->getDataProcessor()->setMultiSliceCount(count);
}

unsigned char* CelexSensorDLL::getMultiSliceBuffer()
{
	if (isMultiSliceEnabled())
	{
		return g_pBufferImage6;
	}
	return NULL;
}

unsigned char* CelexSensorDLL::getMultiSliceDirectionBuffer()
{
	if (isMultiSliceEnabled())
	{
		return g_pBufferImage7;
	}
	return NULL;
}

unsigned char* CelexSensorDLL::getMultiSliceSpeedBuffer()
{
	if (isMultiSliceEnabled())
	{
		return g_pBufferImage8;
	}
	return NULL;
}

//for test purpose: golbal reset length
//address:0x05, mask: 31:12
void CelexSensorDLL::setResetLength(uint32_t value)
{
	//--- excuteCommand("SetMode Global Reset Length"); ---
	value = value << 12;
	FrontPanel::getInstance()->wireIn(0x05, value, 0xFFFFF000);
	FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x05 << "; Value: " << value << "; Mask: " << 0xFFFFF000 << endl;
}

uint32_t CelexSensorDLL::getPageCount()
{
	return m_uiPageCount;
}
