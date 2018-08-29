#include "dataprocessthread.h"
#include <iostream>

using namespace std;

DataProcessThread::DataProcessThread(const std::string &name)
    : XThread(name)
{
}

DataProcessThread::~DataProcessThread()
{
}


void DataProcessThread::addData(unsigned char *data, long length)
{
    m_queueData.push(data, length);
}

void DataProcessThread::clearData()
{
    m_queueData.clear();
}

void DataProcessThread::setSensorMode(emSensorMode mode)
{
    m_queueData.clear();
    m_dataProcessor.setSensorMode(mode);
}

uint32_t DataProcessThread::queueSize()
{
    return m_queueData.size();
}

FPGADataProcessor *DataProcessThread::getDataProcessor()
{
    return &m_dataProcessor;
}

void DataProcessThread::run()
{
    while (m_bRun)
    {
        //cout << "---------- DataProcessThread::run ----------" << endl;
#ifndef _WIN32
        pthread_mutex_lock(&m_mutex);
        while (m_bSuspended)
        {
            pthread_cond_wait(&m_cond, &m_mutex);
        }
        pthread_mutex_unlock(&m_mutex);
#endif
        unsigned char* data = 0;
        long dataLen = 0;
        m_queueData.pop(data, &dataLen);
        if (dataLen > 0)
        {
             m_dataProcessor.processData(data, dataLen);
        }
        delete [] data;
    }
    //Sleep(10);
}



