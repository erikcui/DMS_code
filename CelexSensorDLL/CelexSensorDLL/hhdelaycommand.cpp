#include "hhdelaycommand.h"
#include "frontpanel.h"

HHDelayCommand::HHDelayCommand(const std::string& name)
    : HHCommandBase(name)
{

}

void HHDelayCommand::execute()
{
    FrontPanel::getInstance()->wait(mDuration);
}

void HHDelayCommand::setDuration(int duration)
{
    mDuration = duration;
}

void HHDelayCommand::setValue(uint32_t value)
{
    setDuration(value);
}
