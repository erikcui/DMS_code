#ifndef FPGADATAPROCESSOR_H
#define FPGADATAPROCESSOR_H

#include <queue>
#include <string>
#include <list>
#include <stdint.h>
#include <stdio.h>
#include "hhconstants.h"
#include "dataqueue.h"
#include "celexsensordll.h"

extern unsigned char*   g_pBufferImage0;
extern unsigned char*   g_pBufferImage1;
extern unsigned char*   g_pBufferImage2;
extern unsigned char*   g_pBufferImage3;
extern unsigned char*   g_pBufferImage4;
extern unsigned char*   g_pBufferImage5;

extern unsigned char*   g_pBufferImage6;
extern unsigned char*   g_pBufferImage7;
extern unsigned char*   g_pBufferImage8;

typedef struct PixelData
{
	unsigned int    index;
	unsigned int    value;
    unsigned int    t;
} PixelData;

typedef struct EventRowData
{
	unsigned int row;
	unsigned int t;
	std::queue<int> cols;
} EventRowData;

class SensorData;
class FPGADataProcessor
{
public:
    FPGADataProcessor();
    ~FPGADataProcessor();
    bool setFpnFile(const std::string& fpnFile);
    void processData(unsigned char* data, long length);
    bool processEvent(unsigned char* data); // return true for normal event, false for special event
    void processColEvent(unsigned char* data);
    void createImage();
    void cleanEventBuffer();
    void setSensorMode(emSensorMode mode);
    emSensorMode getSensorMode();
    void generateFPN(std::string filePath);
    void adjustBrightness();
    void setUpperADC(uint32_t value);
    void setLowerADC(uint32_t value);
	//--- for multi-slice ---
	void enableMultiSlice(bool enable);
	bool isMultiSliceEnabled();
    void setTimeSlice(uint32_t msec); //unit: millisecond
	void setOverlapTime(uint32_t msec); //unit: millisecond

    SensorData *getSensorDataObject();

	//--- for clock ---
	void setClockRate(uint32_t value); //unit: MHz
	void setFPGATimeCycle(uint32_t value);

	//--- for multi-slice ---
	void setMultiSliceTime(uint32_t msec);
	void setMultiSliceCount(uint32_t count);

private:
    void generateFPNimpl();
    void adjustBrightnessImpl();
	unsigned int normalizeADC(unsigned int adc);
	//--- for multi-slice
	void createMultiSliceBuffer();
	//
	int calculateDenoiseScore(unsigned char* pBuffer, unsigned char* pBufferPre, unsigned int pos);
	int calculateDenoiseScore(unsigned long* pBuffer, unsigned int pos);
	//
	void pushOverlapQueue(int index, int t, int value);
	void cleanOverlapQueue();

private:
    unsigned char* m_pFullPicBuffer;
    unsigned char* m_pEventPicBuffer;

    long*          m_pFpnGenerationBuffer;
    char*          m_pFpnBuffer;

    //unsigned char* m_pBufferMotion;  //for for ABC mode
	unsigned int* m_pBufferMotion;  //for for ABC mode
    unsigned char* m_pBufferOpticalFlow; //for ABC mode
	unsigned char* m_pLastEventPicBuffer; //for denoise

	SensorData*    m_pSensorData;

	std::string    m_strFpnFilePath;

	emSensorMode   m_emSensorMode;
    //
    int            m_iFpnCalculationTimes;
	unsigned int   m_uiSpecialEventCount;
	unsigned int   m_uiFullPicPixelCount; //for ABC mode
	unsigned int   m_uiOpticalFlowPixelCount; //for ABC mode
	unsigned int   m_uiMotionPixelCount; //for ABC mode
	unsigned int   m_uiLastEventT;
	unsigned int   m_uiEventTCount;
    //
    uint32_t       m_uiUpperADC;
    uint32_t       m_uiLowerADC;
    uint32_t       m_uiTimeSlice;
	uint32_t       m_uiTimeSliceValue; //ms
	uint32_t       m_uiOverlapTime;
	uint32_t       m_uiClockRate;
	uint32_t       m_uiFPGATimeCycle; //Version 2.x: 2^17=131072; Version 1.x: 2^18=262144

	bool           m_bIsGeneratingFPN;
	bool           m_bAdjustBrightness;
	bool           m_bLastEventType;

	std::queue<PixelData>    m_queueOverlapEvent;
	std::queue<PixelData>    m_queueLastOverlapEvent;

	std::ofstream            m_ofstreamLog;

	//for event
	unsigned long            m_ulEventTCount;

	//for multi-slice
	bool                     m_bMultiSliceEnabled;
	uint32_t                 m_uiMultiSliceTime;
	uint32_t                 m_uiMultiSliceCount;
	unsigned long            m_ulMultiSliceTCount;
	std::queue<EventRowData> m_queueMultiSliceRows;
	EventRowData             m_eventRowData;
	unsigned long*           m_pMultiSliceBuffer;
};

#endif // FPGADATAPROCESSOR_H
