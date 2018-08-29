#ifndef HHCOMMAND_H
#define HHCOMMAND_H

#include <string>
#include <stdint.h>

class FrontPanel;

class HHCommandBase
{
public:
    HHCommandBase(const std::string& name);
    ~HHCommandBase();

    std::string name();

    virtual void execute() = 0;
    virtual bool valid();
    virtual void valid(bool bValid);
    virtual std::string error();
    virtual void error(const std::string& error);

    virtual void needsArg(bool bNeed);
    virtual bool needsArg();

    virtual void setValue(uint32_t value) = 0;

    virtual HHCommandBase* clone();

protected:
    std::string     m_strName;

private:
    bool            m_bValid;
    bool            m_bNeedsArg;
    std::string     m_strErrorMessage;
};

#endif // HHCOMMAND_H
