#ifndef HHDELAYCOMMAND_H
#define HHDELAYCOMMAND_H

#include "hhcommand.h"

class HHDelayCommand : public HHCommandBase
{
public:
    HHDelayCommand(const std::string& name);

    void execute();
    void setDuration(int duration);
    virtual void setValue(uint32_t value) override;

private:
    int mDuration;
};

#endif // HHDELAYCOMMAND_H
