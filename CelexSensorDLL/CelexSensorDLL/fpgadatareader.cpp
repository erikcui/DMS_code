#include "fpgadatareader.h"

//Byte 0: {1'b0, X[6:0]}
//Byte 1: {1'b0, C[0], X[8:7], A[3:0]}
unsigned int FPGADataReader::getColumn(unsigned char data[EVENT_SIZE])
{
    unsigned int last7bit = 0;
    last7bit += static_cast<unsigned char>(data[0]) & 0x7F;
    unsigned int result = last7bit + ((static_cast<unsigned char>(data[1]) & 0x30) << 3);
    if ((data[1] & 0x40) != 0)
        result = PIXELS_PER_COL - 1 - result;
    return result;
}

//Byte 0: {1'b1, Y[6:0]}
//Byte 1: {1'b1, Y[9:7], T[3:0]}
unsigned int FPGADataReader::getRow(unsigned char data[EVENT_SIZE])
{
    unsigned int last7bit = 0;
    last7bit += static_cast<unsigned char>(data[0]) & 0x7F;
    return last7bit + ((static_cast<unsigned char>(data[1]) & 0x70) << 3);
}

//Byte 1: {1'b1, Y[9,7], T[3:0]}
//Byte 2: {1'b1, T[10:4]}
//Byte 3: {1'b1, T[17:11]}
unsigned int FPGADataReader::getTimeStamp(unsigned char data[EVENT_SIZE])
{
    unsigned int last4bit = 0;
    last4bit += static_cast<unsigned int>(data[1] & 0x0F);
    unsigned int mid7bit = 0;
    mid7bit += static_cast<unsigned int>(data[2] & 0x7F);
    return last4bit + (mid7bit << 4) + (static_cast<unsigned int>(data[3] & 0x7F) << 11);
}

//Byte 1: {1'b0, C[0], X[8:7], A[3:0]}
//Byte 2: {3'b0, A[8:4]}
unsigned int FPGADataReader::getBrightness(unsigned char data[EVENT_SIZE]) //ADC
{
    unsigned int last4bit = 0;
    last4bit += static_cast<unsigned int>(data[1] & 0x0F);
    return last4bit + (static_cast<unsigned int>(data[2] & 0x1F) << 4);
}

bool FPGADataReader::isForcefirePixel(unsigned char data[EVENT_SIZE]) //column event from force fire, carrying ADC
{
    return ((unsigned char)data[3] & 0x01) == 1;
}

//Byte 0: {1'b0, X[6:0]}
//Byte 1: {1'b0, C[0], X[8:7], A[3:0]}
//Byte 2: {3'b0, A[8:4]}
//Byte 3: {8'b0}
bool FPGADataReader::isColumnEvent(unsigned char data[EVENT_SIZE])
{
    bool result = (((unsigned char)data[0] & 0x80) == 0);
    if (result)
        s_uiEventType = (unsigned int)((unsigned char)data[3] & 0x03);
    return result;
}

//Byte 0: {1'b1, Y[6:0]}
//Byte 1: {1'b1, Y[9:7], T[3:0]}
//Byte 2: {1'b1, T[10:4]}
//Byte 3: {1'b1, T[17:11]}
bool FPGADataReader::isRowEvent(unsigned char data[EVENT_SIZE])
{
    return ((unsigned char)data[0] & 0x80) > 0 && !isSpecialEvent(data);
}

//Byte 0: {8'b1}
//Byte 1: {8'b1}
//Byte 2: {8'b1}
//Byte 3: {8'b1}
bool FPGADataReader::isSpecialEvent(unsigned char data[EVENT_SIZE])
{
    unsigned char a = (unsigned char)data[0];
    unsigned char b = (unsigned char)data[1];
    unsigned char c = (unsigned char)data[2];
    unsigned char d = (unsigned char)data[3];

    bool result = (a == 0xff && b == 0xff && c == 0xff);
    if (result)
    {
        s_uiSpecialEventType = (unsigned int)(d & 0x03);
    }
    return result;
}

unsigned int FPGADataReader::MapTime(unsigned char data[EVENT_SIZE], int count)
{
    if (isRowEvent(data))
    {
		s_uiLastRow = s_uiCurrentRow;
		s_uiLastTFromFPGA = s_uiTFromFPGA;
		//
        s_uiCurrentRow = getRow(data);
        s_uiTFromFPGA = getTimeStamp(data);

        unsigned int temT =  s_uiTFromFPGA + count * HARD_TIMER_CYCLE;
        if(temT >= TIMER_CYCLE)
        {
            s_uiMapT = temT%TIMER_CYCLE;
            count = 0;
        }
        else
        {
            s_uiMapT = temT;
        }
        s_uiTCurrentFrame = s_uiMapT % 750000;
    }
	return count;
}

unsigned int FPGADataReader::s_uiCurrentRow = PIXELS_PER_COL;
unsigned int FPGADataReader::s_uiLastRow = PIXELS_PER_COL;
unsigned int FPGADataReader::s_uiTFromFPGA = 0;
unsigned int FPGADataReader::s_uiLastTFromFPGA = 0;
unsigned int FPGADataReader::s_uiTCurrentFrame = 0;
unsigned int FPGADataReader::s_uiMapT = 0;
unsigned int FPGADataReader::s_uiEventType = 0;
unsigned int FPGADataReader::s_uiSpecialEventType = 0;
