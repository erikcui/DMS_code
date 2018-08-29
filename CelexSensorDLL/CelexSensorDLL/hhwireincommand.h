#ifndef HHWIREINCOMMAND_H
#define HHWIREINCOMMAND_H

#include <stdint.h>
#include "hhcommand.h"

class FrontPanel;

class HHWireinCommand : public HHCommandBase
{
public:
    HHWireinCommand(const std::string& name);

    virtual void execute() override;

    virtual void setValue(uint32_t value) override;
    void setAddress(uint32_t address);
    void setMask(uint32_t mask);

    virtual HHCommandBase* clone() override;

private:
    uint32_t     m_uiAddress;
    uint32_t     m_uiValue;
    uint32_t     m_uiMask;
};

#endif // HHWIREINCOMMAND_H
