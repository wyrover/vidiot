#include "RootCommand.h"

namespace command {

RootCommand::RootCommand()
:   wxCommand(true)
,   mCommandName(_("Unnamed command"))
{
}

RootCommand::~RootCommand()
{
}

wxString RootCommand::GetName() const
{
    return mCommandName;
}

} // namespace