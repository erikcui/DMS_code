#include "hhcommand.h"

HHCommandBase::HHCommandBase(const std::string& name)
    : m_strName(name)
    , m_bValid(true)
    , m_bNeedsArg(false)
    , m_strErrorMessage("")
{

}

HHCommandBase::~HHCommandBase()
{
}

std::string HHCommandBase::name()
{
    return m_strName;
}

void HHCommandBase::valid(bool bValid)
{
    m_bValid = bValid;
}

bool HHCommandBase::valid()
{
    return m_bValid;
}

std::string HHCommandBase::error()
{
    return m_strErrorMessage;
}

void HHCommandBase::error(const std::string& error)
{
    m_strErrorMessage = error;
}

void HHCommandBase::needsArg(bool bNeed)
{
    m_bNeedsArg = bNeed;
}

bool HHCommandBase::needsArg()
{
    return m_bNeedsArg;
}

HHCommandBase* HHCommandBase::clone()
{
    return NULL;
}
