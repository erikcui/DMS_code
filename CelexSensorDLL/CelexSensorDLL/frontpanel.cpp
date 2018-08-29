#include "frontpanel.h"

FrontPanel* FrontPanel::spFrontPanel = NULL;

void FrontPanel::initializeFPGA(const string &bitfileName)
{
    cout << "Begin initialize FPGA." << endl;
    if (!myxem->IsOpen())
    {
        if (okCFrontPanel::NoError != myxem->OpenBySerial())
        {
            cout << "FrontPanel Device not found..." << endl;
            return;
        }
    }
    std::string modelString = myxem->GetBoardModelString(myxem->GetBoardModel()); //first device model type
    cout << "Device type: " << modelString << endl;

    // Configure the PLL appropriately
    myxem->LoadDefaultPLLConfiguration();

    cout << "Device serial number: " << myxem->GetSerialNumber() << endl;
    cout << "Device ID string: " << myxem->GetDeviceID() << endl;

    if (okCFrontPanel::NoError != myxem->OpenBySerial(myxem->GetSerialNumber()))
    {
        cout << "Can't open Serial." << endl;
        return;
    }
    else
    {
        cout << "Serial opens successfully." << endl;
    }

    XBase base;
    std::string bitfilePath = base.getApplicationDirPath(); //+ "/" + bitfileName;
#ifdef _WIN32
    bitfilePath += "\\";
#endif
    bitfilePath += bitfileName;
    if (!base.isFileExists(bitfilePath))
    {
        return;
    }
    if (okCFrontPanel::NoError != myxem->ConfigureFPGA(bitfilePath))
    {
        cout << "Fail to load *.bit file!" << endl;
        return;
    }
    mReady = true;
    cout << "FPGA Ready." << endl;

    //operation_init_sensor_config();
    //operation_reset_dereset_ALL();
}

void FrontPanel::uninitializeFPGA()
{
    myxem->ResetFPGA();
    myxem->Close();
}

bool FrontPanel::wireIn(uint32_t address, uint32_t value, uint32_t mask)
{
    if (okCFrontPanel::NoError != myxem->SetWireInValue(address, value, mask))
    {
        //sendMessage("WireIn error", EMessageType::error);
        return false;
    }
    myxem->UpdateWireIns();
    return true;
}

void FrontPanel::wireOut(uint32_t address, uint32_t mask, uint32_t *pValue)
{
    myxem->UpdateWireOuts();
    *pValue = (myxem->GetWireOutValue(address)) & mask;
}

long FrontPanel::blockPipeOut(uint32_t address, int blockSize, long length, unsigned char *data)
{
    //format is addr: blockSize: byte number: buffer
    long dataLen = myxem->ReadFromBlockPipeOut(address, blockSize, length, data);
    return dataLen;
}
