#ifndef DATAPROCESSTHREAD_H
#define DATAPROCESSTHREAD_H

#include "dataqueue.h"
#include "fpgadataprocessor.h"
#include "xthread.h"

class DataProcessThread : public XThread
{
public:
    DataProcessThread(const std::string& name = "DataProcessThread");
    ~DataProcessThread();

    void addData(unsigned char* data, long length);
    void clearData();
    void setSensorMode(emSensorMode mode);
    uint32_t queueSize();
    FPGADataProcessor* getDataProcessor();

protected:
    void run() override;

private:
    DataQueue            m_queueData;
    FPGADataProcessor    m_dataProcessor;
};

#endif // DATAPROCESSTHREAD_H
