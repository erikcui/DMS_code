#ifndef HHXMLREADER_H
#define HHXMLREADER_H

#include <string>
#include <vector>
#include <string>
#include <stdint.h>

class TiXmlDocument;
class QDomDocument;
class HHCommandBase;
class HHSequence;
class HHSequenceMgr;

class HHXmlReader
{
public:
    HHXmlReader();
    ~HHXmlReader();

    bool parse(const std::string& filename, TiXmlDocument* pDom);
    bool importCommands(std::vector<HHCommandBase*>& commandList, TiXmlDocument* pDom);
    bool importSequences(HHSequenceMgr* pSeqMgr, std::vector<HHSequence*>& sequenceList, TiXmlDocument* pDom);
    bool importSliders(HHSequenceMgr* pSeqMgr, std::vector<HHSequence*>& sliderList, TiXmlDocument* pDom);

 private:
    bool getNumber(const std::string& text, uint32_t* pNumber);
};

#endif // HHXMLREADER_H
