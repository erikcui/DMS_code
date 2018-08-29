#ifndef XBASE_H
#define XBASE_H

#include <string>

using namespace std;

class XBase
{
public:
    XBase();
    ~XBase();

    std::string getApplicationDirPath();
    std::string getDateTimeStamp();
    int getTimeStamp();
    bool isFileExists(std::string filePath);
};

#endif // XBASE_H
