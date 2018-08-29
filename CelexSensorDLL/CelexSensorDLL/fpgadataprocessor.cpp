#include "fpgadataprocessor.h"
#include "hhconstants.h"
#include "fpgadatareader.h"
#include "xbase.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>
#ifndef _WIN32
#include <cstring>
#endif

#define PI (atan(1.0) * 4.0)

//#define _LOG_ENABLE_

unsigned char* g_pBufferImage0 = NULL;
unsigned char* g_pBufferImage1 = NULL;
unsigned char* g_pBufferImage2 = NULL;
unsigned char* g_pBufferImage3 = NULL;
unsigned char* g_pBufferImage4 = NULL;
unsigned char* g_pBufferImage5 = NULL;

unsigned char* g_pBufferImage6 = NULL;
unsigned char* g_pBufferImage7 = NULL;
unsigned char* g_pBufferImage8 = NULL;

FPGADataProcessor::FPGADataProcessor()
	: m_bIsGeneratingFPN(false)
	, m_bAdjustBrightness(false)
	, m_iFpnCalculationTimes(0)
	, m_uiSpecialEventCount(0)
	, m_uiFullPicPixelCount(0)
	, m_uiOpticalFlowPixelCount(0)
	, m_uiMotionPixelCount(0)
	, m_uiUpperADC(511)
	, m_uiLowerADC(0)
	, m_uiTimeSlice(750000)
	, m_uiTimeSliceValue(60)
	, m_uiClockRate(25)
	, m_uiFPGATimeCycle(131072)
	, m_uiOverlapTime(0)
	, m_uiLastEventT(0)
	, m_uiEventTCount(0)
    , m_emSensorMode(EventMode)
    , m_pEventPicBuffer(NULL)
	, m_pLastEventPicBuffer(NULL)
	, m_bMultiSliceEnabled(false)
	, m_ulEventTCount(0)
	, m_ulMultiSliceTCount(0)
{
    m_pFullPicBuffer = new unsigned char[PIXELS_NUMBER];
	m_pEventPicBuffer = new unsigned char[PIXELS_NUMBER];

    m_pFpnGenerationBuffer = new long[PIXELS_NUMBER];

    //m_pBufferMotion = new unsigned char[PIXELS_NUMBER];
	m_pBufferMotion = new unsigned int[PIXELS_NUMBER];

	m_pBufferOpticalFlow = new unsigned char[PIXELS_NUMBER];
    //
	m_pMultiSliceBuffer = new unsigned long[PIXELS_NUMBER];
	//
    m_pFpnBuffer = new char[PIXELS_NUMBER];
    for (int i = 0; i < PIXELS_NUMBER; ++i)
    {
		m_pFullPicBuffer[i] = 0;
		m_pEventPicBuffer[i] = 0;
        m_pFpnBuffer[i] = 0;
        m_pBufferMotion[i] = 0;
		m_pBufferOpticalFlow[i] = 0;
		m_pMultiSliceBuffer[i] = 0;
    }
    g_pBufferImage0 = new unsigned char[PIXELS_NUMBER];
    g_pBufferImage1 = new unsigned char[PIXELS_NUMBER];
	g_pBufferImage2 = new unsigned char[PIXELS_NUMBER];
	g_pBufferImage3 = new unsigned char[PIXELS_NUMBER];
	g_pBufferImage4 = new unsigned char[PIXELS_NUMBER];
	g_pBufferImage5 = new unsigned char[PIXELS_NUMBER];

	g_pBufferImage6 = new unsigned char[PIXELS_NUMBER];
	g_pBufferImage7 = new unsigned char[PIXELS_NUMBER];
	g_pBufferImage8 = new unsigned char[PIXELS_NUMBER];

    m_pSensorData = new SensorData;

#ifdef _LOG_ENABLE_
	m_ofstreamLog.open("./log.txt");
#endif
}

FPGADataProcessor::~FPGADataProcessor()
{
    if (m_pFullPicBuffer)
        delete[] m_pFullPicBuffer;
	if (m_pEventPicBuffer)
		delete[] m_pEventPicBuffer;
    if (m_pFpnBuffer)
        delete[] m_pFpnBuffer;
    if (m_pFpnGenerationBuffer)
        delete[] m_pFpnGenerationBuffer;

	if (g_pBufferImage0)
		delete[] g_pBufferImage0;

#ifdef _LOG_ENABLE_
	m_ofstreamLog.close();
#endif
}

bool FPGADataProcessor::setFpnFile(const std::string &fpnFile)
{
    int index = 0;
    std::ifstream in;
    in.open(fpnFile.c_str());
    if (!in.is_open())
    {
        return false;
    }
    std::string line;
    while (!in.eof() && index < PIXELS_NUMBER)
    {
        in >> line;
        m_pFpnBuffer[index] = atoi(line.c_str());
        index++;
    }
    if (index != PIXELS_NUMBER)
        return false;

    return true;
}

void FPGADataProcessor::processData(unsigned char *data, long length)
{
    if (!data)
    {
        return;
    }
    //--------------- Align data ---------------
    long i = 0;
    while (i + 1 < length)
    {
        if ((data[i] & 0x80) > 0 && (data[i+1] & 0x80) == 0x00)
        {
            i = i - 3;
            // init sRow and sT
            processEvent(&data[i]);
            break; // aligned
        }
        ++i;
    }
    //
    for (i = i + EVENT_SIZE; i + 11 < length; i = i + EVENT_SIZE)
    {
        bool isSpecialEvent = !(processEvent(&data[i]));
        if (isSpecialEvent)
        {
			if (m_bAdjustBrightness)
			{
				adjustBrightnessImpl();
			}
			else if (m_bIsGeneratingFPN)
			{
				generateFPNimpl();
			}
			else
			{
				createImage();
				if (m_emSensorMode == FullPictureMode)
				{
					m_pSensorData->setFullPicBuffer(g_pBufferImage0);
				}
				else if (m_emSensorMode == EventMode)
				{
					m_pSensorData->setFullPicBuffer(g_pBufferImage0);
					m_pSensorData->setEventPicBuffer(g_pBufferImage1);
					m_pSensorData->setEventPicBufferGray(g_pBufferImage2);
					m_pSensorData->setEventPicBufferSuperimposed(g_pBufferImage3);
					m_pSensorData->setEventPicBufferDenoisedBinary(g_pBufferImage4);
					m_pSensorData->setEventPicBufferDenoisedGray(g_pBufferImage5);
					m_pSensorData->setEventPicMultiSlice(g_pBufferImage6);
					m_pSensorData->setEventPicMultiSliceDirection(g_pBufferImage7);
					m_pSensorData->setEventPicMultiSliceSpeed(g_pBufferImage8);
				}
				else if (m_emSensorMode == FullPic_Event_Mode)
				{
					m_pSensorData->setFullPicBuffer(g_pBufferImage0);
					m_pSensorData->setEventPicBuffer(g_pBufferImage1);
					m_pSensorData->setEventPicBufferGray(g_pBufferImage2);
					m_pSensorData->setEventPicBufferSuperimposed(g_pBufferImage3);
				}
				m_pSensorData->notify(Observer::emPicFrameBufferData);
				m_pSensorData->notify(Observer::emSpecialEvent);
			}
			if (m_uiOverlapTime > 0)
				cleanOverlapQueue();
			cleanEventBuffer();

			m_uiFullPicPixelCount = 0;
			m_uiOpticalFlowPixelCount = 0;
			m_uiMotionPixelCount = 0;
        }
    }
}

bool FPGADataProcessor::processEvent(unsigned char *data)
{
	if (FPGADataReader::isSpecialEvent(data))
	{
		++m_uiSpecialEventCount;
		//cout << FPGADataReader::getTFromFPGA() << endl;
		if (EventMode != m_emSensorMode)
		{
			if (m_emSensorMode == FullPic_Event_Mode)
			{
				//cout << "FullPicPixelCount = " << m_uiFullPicPixelCount << ", OpticalFlowPixelCount = " << m_uiOpticalFlowPixelCount << ", MotionPixelCount = " << m_uiMotionPixelCount << endl; 					
			}
			return false;
		}
	}
	else if (FPGADataReader::isRowEvent(data))
	{
		m_uiSpecialEventCount = FPGADataReader::MapTime(data, m_uiSpecialEventCount);
		if (m_emSensorMode == EventMode)
		{
			unsigned int t = FPGADataReader::getTFromFPGA();
			//cout << t << endl;
			unsigned int tLast = FPGADataReader::getLastTFromFPGA();
			int diffT = t - tLast;
			if (diffT < 0)
			{
				//cout << "t = " << t << ", tLast = " << tLast << endl;
				diffT = diffT + m_uiFPGATimeCycle;
			}
			//
			if (m_bMultiSliceEnabled)
			{
				if (m_eventRowData.cols.size() > 0)
				{
					m_eventRowData.row = FPGADataReader::getLastRow();
					m_eventRowData.t = diffT;
					m_queueMultiSliceRows.push(m_eventRowData);
					m_ulMultiSliceTCount += diffT;

					while (m_eventRowData.cols.size() > 0)
					{
						m_eventRowData.cols.pop();
					}
					//cout << "-----------------------" << m_ulMultiSliceTCount << "  " << diffT << endl;
				}
				while (m_ulMultiSliceTCount > m_uiMultiSliceTime)
				{
					EventRowData popData = m_queueMultiSliceRows.front();
					m_queueMultiSliceRows.pop();
					m_ulMultiSliceTCount -= popData.t;
				}
			}
			m_ulEventTCount += diffT;
			if (m_ulEventTCount > m_uiTimeSlice)
			{
				//cout << m_ulEventTCount << "  " << m_uiTimeSlice << endl;
				m_ulEventTCount = 0;
				return false;
			}
		}
		else if (m_emSensorMode == FullPic_Event_Mode)
		{
			if (FPGADataReader::getEventType() == 0)
			{
				unsigned int t = FPGADataReader::getTFromFPGA();
				//cout << t << endl;
				unsigned int tLast = FPGADataReader::getLastTFromFPGA();
				int diffT = t - tLast;
				if (diffT < 0)
				{
					//cout << "t = " << t << ", tLast = " << tLast << endl;
					diffT = diffT + m_uiFPGATimeCycle;
				}
				//
				if (m_eventRowData.cols.size() > 0)
				{
					m_eventRowData.row = FPGADataReader::getLastRow();
					m_eventRowData.t = diffT;
					m_queueMultiSliceRows.push(m_eventRowData);
					m_ulMultiSliceTCount += diffT;

					while (m_eventRowData.cols.size() > 0)
					{
						m_eventRowData.cols.pop();
					}
					//cout << "-----------------------" << m_ulMultiSliceTCount << "  " << diffT << endl;
				}
			}
		}
	}
	else if (FPGADataReader::isColumnEvent(data))
	{
		processColEvent(data);
	}
    return true;
}

SensorData *FPGADataProcessor::getSensorDataObject()
{
    return m_pSensorData;
}

void FPGADataProcessor::setClockRate(uint32_t value)
{
	m_uiClockRate = value;
	m_uiTimeSlice = m_uiClockRate * 1000 * m_uiTimeSliceValue / 2; //25000 * msec;
}

void FPGADataProcessor::setFPGATimeCycle(uint32_t value)
{
	m_uiFPGATimeCycle = value;
}

void FPGADataProcessor::setMultiSliceTime(uint32_t msec)
{
	m_uiMultiSliceTime = m_uiClockRate * 1000 * msec / 2; //25000 * msec;
}

void FPGADataProcessor::setMultiSliceCount(uint32_t count)
{
	m_uiMultiSliceCount = count;
}

void FPGADataProcessor::processColEvent(unsigned char* data)
{
    unsigned int row = FPGADataReader::getCurrentRow();
    unsigned int col = FPGADataReader::getColumn(data);
    unsigned int adc = FPGADataReader::getBrightness(data);
    unsigned int t   = FPGADataReader::getTFromFPGA();
    if (row < PIXELS_PER_ROW && col < PIXELS_PER_COL)
    {
        // normalize to 0~255
        adc = normalizeADC(adc);
        int index = row * PIXELS_PER_COL + col;

        if (m_emSensorMode == FullPictureMode)
        {
            m_pFullPicBuffer[index] = adc;
            t = 0;
        }
        else if (m_emSensorMode == EventMode)
        {
            m_pFullPicBuffer[index] = adc;
			m_pEventPicBuffer[index] += 1;
			if (m_uiOverlapTime > 0)
			{
				pushOverlapQueue(index, FPGADataReader::getTFromFPGA(), 1);
			}
			if (m_bMultiSliceEnabled)
			{
				m_eventRowData.cols.push(col);
			}
        }
		else if (m_emSensorMode == FullPic_Event_Mode)
        {
            unsigned int type = FPGADataReader::getEventType();
            if (0 == type) //Motion
            {
                m_pBufferMotion[index] += 1;
                m_uiMotionPixelCount++;
            }
            else if (1 == type) //Full Picture
            {
                m_pFullPicBuffer[index] = adc;
                m_uiFullPicPixelCount++;
            }
            else if (2 == type) //Optical Flow
            {
                m_pBufferOpticalFlow[index] = adc; //for optical-flow A is T
                m_uiOpticalFlowPixelCount++;
            }
			//
			m_eventRowData.cols.push(col);
        }
        if (m_pSensorData)
        {
            m_pSensorData->setPixelData(col, row, adc, t);
            m_pSensorData->notify(Observer::emPixelData);
        }
    }
}

void FPGADataProcessor::createImage()
{
	if (m_emSensorMode == EventMode)
	{
		if (m_uiOverlapTime > 0 && m_queueLastOverlapEvent.size() > 0)
		{
			PixelData data1 = m_queueLastOverlapEvent.back();
			unsigned int TCount = data1.t - data1.t / (m_uiTimeSlice / m_uiOverlapTime);
			//cout << "TCount = " << TCount << endl;
			while (m_queueLastOverlapEvent.size() > 0)
			{
				PixelData data = m_queueLastOverlapEvent.front();
				if (data.t > TCount)
				{
					m_pEventPicBuffer[data.index] += 1;
				}
				m_queueLastOverlapEvent.pop();
			}
		}
		m_queueLastOverlapEvent = m_queueOverlapEvent;

		if (m_bMultiSliceEnabled)
		{
			createMultiSliceBuffer();
			for (int i = 0; i < PIXELS_NUMBER; ++i)
			{
				//--- denoised pic ---
				if (m_pMultiSliceBuffer[i] > 0)
				{
					if (0 == calculateDenoiseScore(m_pMultiSliceBuffer, i))
						m_pMultiSliceBuffer[i] = 0;
				}
			}
		}
	}
	else if (m_emSensorMode == FullPic_Event_Mode)
	{
		unsigned int uiTPerSlice = m_ulMultiSliceTCount / m_uiMultiSliceCount;
		unsigned int uiTCount = 0;
		unsigned int slice = 1;
		unsigned long ulAccumulatedT = 0;

		while (!m_queueMultiSliceRows.empty())
		{
			EventRowData data = m_queueMultiSliceRows.front();
			m_queueMultiSliceRows.pop();
			uiTCount += data.t;
			ulAccumulatedT += data.t;
			//cout << "      row = " << data.row << ", size = " << data.cols.size() << "  " << data.t << endl;
			while (!data.cols.empty())
			{
				int col = data.cols.front();
				data.cols.pop();
				int index = data.row * 768 + col;
				m_pMultiSliceBuffer[PIXELS_NUMBER - index - 1] = slice;
			}
			if (uiTCount > uiTPerSlice && slice < 254)
			{
				//cout << "-------------------" << "slice = " << slice << "  " << ulAccumulatedT << endl;
				slice++;
				uiTCount = 0;
			}
		}
		//cout << "m_ulMultiSliceTCount = " << m_ulMultiSliceTCount << "  " << ulAccumulatedT << "  " << slice << "  " << uiTPerSlice << endl;
		m_ulMultiSliceTCount = 0;
		//add bu heguoping denoise
		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			//--- denoised pic ---
			if (m_pMultiSliceBuffer[i] > 0)
			{
				if (0 == calculateDenoiseScore(m_pMultiSliceBuffer, i))
					m_pMultiSliceBuffer[i] = 0;
			}
		}
	}
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		//--- full picture buffer ---
		int value = m_pFullPicBuffer[i] - m_pFpnBuffer[i]; //Subtract FPN
		if (value < 0)
			value = 0;
		if (value > 255)
			value = 255;
		g_pBufferImage0[PIXELS_NUMBER - i - 1] = value;
		g_pBufferImage3[PIXELS_NUMBER - i - 1] = value;

		//--- event picture buffer ---
		if (m_emSensorMode == EventMode)
		{
			g_pBufferImage1[PIXELS_NUMBER - i - 1] = m_pEventPicBuffer[i] > 0 ? 255 : 0; //binary pic
			g_pBufferImage2[PIXELS_NUMBER - i - 1] = m_pEventPicBuffer[i] > 0 ? value : 0; //gray pic
			g_pBufferImage3[PIXELS_NUMBER - i - 1] = m_pEventPicBuffer[i] > 0 ? 255 : value; //superimposed pic

			//--- denoised pic ---
			int score = m_pEventPicBuffer[i];
			if (score > 0)
				score = calculateDenoiseScore(m_pEventPicBuffer, m_pLastEventPicBuffer, i);
			else
				score = 0;

			g_pBufferImage4[PIXELS_NUMBER - i - 1] = score; //denoised binary pic
			g_pBufferImage5[PIXELS_NUMBER - i - 1] = score > 0 ? value : 0; //denoised gray pic

			if (m_bMultiSliceEnabled)
			{
				g_pBufferImage6[i] = m_pMultiSliceBuffer[i];

				int row = i / PIXELS_PER_COL;
				int col = i % PIXELS_PER_COL;
				int Gx = 0, Gy = 0;

				if (col == 0 || col == 767)
					Gx = 0;
				else
					Gx = m_pMultiSliceBuffer[i + 1] - m_pMultiSliceBuffer[i - 1];

				if (row == 0 || row == 639)
					Gy = 0;
				else
					Gy = m_pMultiSliceBuffer[i + 768] - m_pMultiSliceBuffer[i - 768];

				int theta = 0;
				if (Gx == 0 && Gy == 0)
				{
					theta = 0;
				}
				else
				{
					if (Gx == 0)
					{
						if (Gy > 0)
							theta = 90;
						else
							theta = 270;
					}
					else
					{
						theta = atan2(Gy, Gx) * 180 / PI;
					}
				}
				if (theta < 0)
					theta += 360;
				g_pBufferImage7[i] = theta * 255 / 360;

				int value1 = sqrt(Gx*Gx + Gy*Gy);
				if (value1 > 255)
					value1 = 255;
				g_pBufferImage8[i] = value1;
			}
		}
		else if (m_emSensorMode == FullPic_Event_Mode)
		{
			//g_pBufferImage1[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 0 ? 255 : 0;
			g_pBufferImage1[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 10 ? 255 : 0;//times count 
			g_pBufferImage2[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 0 ? value : 0;
			g_pBufferImage3[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 0 ? 255 : value;
			m_pBufferMotion[i] = 0;
			g_pBufferImage6[i] = m_pMultiSliceBuffer[i];
		}
	}
#if 1
	if (m_emSensorMode == FullPic_Event_Mode)
	{
		//filter
		int col = 0;
		int row = 0;
		//for (int i = 0; i < PIXELS_NUMBER; ++i)
		//{
		//	if (col <= 0 || col >= PIXELS_PER_COL - 1 || row <= 0 || row >= PIXELS_PER_ROW - 1)
		//	{
		//		// edge point
		//		g_pBufferImage1[PIXELS_NUMBER - i - 1] = 0;
		//	}
		//	else
		//	{
		//		int k;
		//		unsigned char x1 = g_pBufferImage1[i - 1];
		//		unsigned char x2 = g_pBufferImage1[i + 1];
		//		unsigned char y1 = g_pBufferImage1[i - PIXELS_PER_COL];
		//		unsigned char y2 = g_pBufferImage1[i + PIXELS_PER_COL];
		//		if ((x1 == 0 && x2 == 0) || (y1 == 0 && y2 == 0))
		//		{
		//			g_pBufferImage1[i] = 0;
		//		}

		//	}
		//	++col;
		//	if (col >= PIXELS_PER_COL)
		//	{
		//		++row;
		//		col = 0;
		//	}
		//}
	}
#endif
	if (m_emSensorMode == EventMode)
	{
		if (NULL == m_pLastEventPicBuffer)
		{
			m_pLastEventPicBuffer = new unsigned char[PIXELS_NUMBER];
		}
		memcpy(m_pLastEventPicBuffer, m_pEventPicBuffer, PIXELS_NUMBER);
	}
}

void FPGADataProcessor::cleanEventBuffer()
{
	//if (m_emSensorMode == EventMode)
	{
		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			m_pEventPicBuffer[i] = 0;
			m_pMultiSliceBuffer[i] = 0;	
		}
	}
}

void FPGADataProcessor::setSensorMode(emSensorMode mode)
{
    m_emSensorMode = mode;
}

emSensorMode FPGADataProcessor::getSensorMode()
{
    return m_emSensorMode;
}

void FPGADataProcessor::generateFPN(std::string filePath)
{
    m_bIsGeneratingFPN = true;
    m_iFpnCalculationTimes = FPN_CALCULATION_TIMES;
    m_strFpnFilePath = filePath;
}

void FPGADataProcessor::adjustBrightness()
{
    m_bAdjustBrightness = (!m_bAdjustBrightness);
}

void FPGADataProcessor::setUpperADC(uint32_t value)
{
    m_uiUpperADC = value;
}

void FPGADataProcessor::setLowerADC(uint32_t value)
{
    m_uiLowerADC = value;
}

void FPGADataProcessor::enableMultiSlice(bool enable)
{
	m_bMultiSliceEnabled = enable;
}

bool FPGADataProcessor::isMultiSliceEnabled()
{
	return m_bMultiSliceEnabled;
}

void FPGADataProcessor::setTimeSlice(uint32_t msec)
{
	m_uiTimeSliceValue = msec;
    m_uiTimeSlice = m_uiClockRate * 1000 * msec / 2;
	if (m_uiOverlapTime > m_uiTimeSlice)
		m_uiOverlapTime = m_uiTimeSlice;
}

void FPGADataProcessor::setOverlapTime(uint32_t msec)
{
	m_uiOverlapTime = m_uiClockRate * 1000 * msec / 2; //25000 * msec;
	if (m_uiOverlapTime > m_uiTimeSlice)
		m_uiOverlapTime = m_uiTimeSlice;
}

void FPGADataProcessor::generateFPNimpl()
{
    for (int i = 0; i < PIXELS_NUMBER; ++i)
    {
        if (m_iFpnCalculationTimes == FPN_CALCULATION_TIMES)
        {
            m_pFpnGenerationBuffer[i] = m_pFullPicBuffer[i];
        }
        else
        {
            m_pFpnGenerationBuffer[i] += m_pFullPicBuffer[i];
        }
    }
    --m_iFpnCalculationTimes;

    if (m_iFpnCalculationTimes <= 0)
    {
        m_bIsGeneratingFPN = false;
        std::ofstream ff;
        if (m_strFpnFilePath.empty())
        {
            XBase base;
            std::string filePath = base.getApplicationDirPath();
            filePath += "\\FPN.txt";
            // output the FPN file now
            ff.open(filePath.c_str());
        }
        else
        {
            ff.open(m_strFpnFilePath.c_str());
        }
        if (!ff)
            return;
        long total = 0;
        for (int i = 0; i < PIXELS_NUMBER; ++i)
        {
            m_pFpnGenerationBuffer[i] = m_pFpnGenerationBuffer[i] / FPN_CALCULATION_TIMES;
            total += m_pFpnGenerationBuffer[i];
        }
        int avg = total / PIXELS_NUMBER;
        for (int i = 0; i < PIXELS_NUMBER; ++i)
        {
            int d = m_pFpnGenerationBuffer[i] - avg;
            ff << d;
            ff << "\n";
        }
        ff.close();
    }
}

void FPGADataProcessor::adjustBrightnessImpl()
{
    long total = 0;
    for (int i = 0; i < PIXELS_NUMBER; ++i)
    {
        total += m_pFullPicBuffer[i];
    }
    long avg = total / PIXELS_NUMBER;

    if (avg < 118)
    {
        //emit commandCallback("up_brightness");
    }
    if (avg > 138)
    {
        //emit commandCallback("down_brightness");
    }
}

// normalize to 0~255
unsigned int FPGADataProcessor::normalizeADC(unsigned int adc)
{
    int brightness = adc;
    if (adc < m_uiLowerADC)
        brightness = 0;
    else if (adc > m_uiUpperADC)
        brightness = 255;
    else
        brightness = 255 * (adc - m_uiLowerADC) / (m_uiUpperADC - m_uiLowerADC);
    return brightness;
}

void FPGADataProcessor::createMultiSliceBuffer()
{
	std::queue<EventRowData> queueData = m_queueMultiSliceRows;
	unsigned int uiTPerSlice = m_uiMultiSliceTime / m_uiMultiSliceCount;
	unsigned int uiTCount = 0;
	unsigned int slice = 1;
	unsigned long ulAccumulatedT = 0;

	while (!queueData.empty())
	{
		EventRowData data = queueData.front();
		queueData.pop();
		uiTCount += data.t;
		ulAccumulatedT += data.t;
		//cout << "      row = " << data.row << ", size = " << data.cols.size() << "  " << data.t << endl;
		while (!data.cols.empty())
		{
			int col = data.cols.front();
			data.cols.pop();
			int index = data.row * 768 + col;
			m_pMultiSliceBuffer[PIXELS_NUMBER - index - 1] = slice;
		}
		if (uiTCount > uiTPerSlice && slice < 254)
		{
			//cout << "-------------------" << "slice = " << slice << "  " << ulAccumulatedT << endl;
			slice++;
			uiTCount = 0;
		}
	}
	//cout << "m_ulMultiSliceTCount = " << m_ulMultiSliceTCount << "  " << ulAccumulatedT << "  " << slice << "  " << uiTPerSlice << endl;	
#if 1
	//filter
	int col = 0;
	int row = 0;
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		if (col <= 0 || col >= PIXELS_PER_COL - 1 || row <= 0 || row >= PIXELS_PER_ROW - 1)
		{
			// edge point
			m_pMultiSliceBuffer[PIXELS_NUMBER - i - 1] = 0;
		}
		else
		{
			int k;
			unsigned char x1 = m_pMultiSliceBuffer[i - 1];
			unsigned char x2 = m_pMultiSliceBuffer[i + 1];
			unsigned char y1 = m_pMultiSliceBuffer[i - PIXELS_PER_COL];
			unsigned char y2 = m_pMultiSliceBuffer[i + PIXELS_PER_COL];
			if ((x1 == 0 && x2 == 0) ||(y1 == 0 && y2 == 0))
			{
				m_pMultiSliceBuffer[i] = 0;
			}

		}
		++col;
		if (col >= PIXELS_PER_COL)
		{
			++row;
			col = 0;
		}
	}
#endif
}

int FPGADataProcessor::calculateDenoiseScore(unsigned char* pBuffer, unsigned char* pBufferPre, unsigned int pos)
{
	if (NULL == pBuffer || NULL == pBufferPre)
	{
		return 255;
	}
	int row = pos / PIXELS_PER_COL;
	int col = pos % PIXELS_PER_COL;

	int count1 = 0;
	int count2 = 0;
	int count3 = 0;
	int count4 = 0;
	for (int i = row - 1; i < row + 2; ++i) //8 points
	{
		for (int j = col - 1; j < col + 2; ++j)
		{
            int index = i * PIXELS_PER_COL + j;
            if (index < 0 || index == pos || index >= PIXELS_NUMBER)
				continue;
			if (pBuffer[index] > 0)
				++count1;
			else
				++count2;
		}
	}
	int index1 = (row - 1)*PIXELS_PER_COL + col;
	int index2 = row*PIXELS_PER_COL + col - 1;
	int index3 = row*PIXELS_PER_COL + col + 1;
	int index4 = (row + 1)*PIXELS_PER_COL + col;
	int aa[4] = { index1, index2, index3, index4 };
	for (int i = 0; i < 4; ++i)
	{
        if (aa[i] < 0 || aa[i] >= PIXELS_NUMBER)
            continue;
        if (pBufferPre[aa[i]] > 0)
			++count3;
		else
			++count4;
	}
	if (count1 >= count2 || count3 >= count4)
		return 255;
	else
		return 0;
}

int FPGADataProcessor::calculateDenoiseScore(unsigned long* pBuffer, unsigned int pos)
{
	if (NULL == pBuffer)
	{
		return 0;
	}
	int row = pos / PIXELS_PER_COL;
	int col = pos % PIXELS_PER_COL;

	int count1 = 0, count2 = 0;
	int index1 = (row - 1)*PIXELS_PER_COL + col;
	int index2 = row*PIXELS_PER_COL + col - 1;
	int index3 = row*PIXELS_PER_COL + col + 1;
	int index4 = (row + 1)*PIXELS_PER_COL + col;
	int aa[4] = { index1, index2, index3, index4 };
	for (int i = 0; i < 4; ++i)
	{
		if (aa[i] < 0 || aa[i] >= PIXELS_NUMBER)
			continue;
		if (pBuffer[aa[i]] > 0)
			++count1;
		else
			++count2;
	}
	if (count1 >= count2)
		return 255;
	else
		return 0;
}

void FPGADataProcessor::pushOverlapQueue(int index, int t, int value)
{
	PixelData pixelData;
	pixelData.index = index;
	pixelData.value = value;
	int diffT = t - m_uiLastEventT;
	if (diffT < 0)
	{
		pixelData.t = m_uiEventTCount + t;
#ifdef _LOG_ENABLE_
		m_ofstreamLog << "------------------------------\n";
#endif
	}	
	else
	{
		pixelData.t = m_uiEventTCount + diffT;
	}
	m_uiLastEventT = t;
	m_uiEventTCount = pixelData.t;
	m_queueOverlapEvent.push(pixelData);

#ifdef _LOG_ENABLE_
	m_ofstreamLog << m_uiEventTCount;
	m_ofstreamLog << "\n";
#endif
}

void FPGADataProcessor::cleanOverlapQueue()
{
	while (m_queueOverlapEvent.size() > 0)
	{
		m_queueOverlapEvent.pop();
	}
	m_uiEventTCount = 0;
#ifdef _LOG_ENABLE_
	m_ofstreamLog << "-------------------------------------------\n";
#endif
}
