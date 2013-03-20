#include "Combiner.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Combiner::Combiner()
    :   RootCommand()
{
}

Combiner::~Combiner()
{
    while (!mCommands.empty())
    {
        delete mCommands.front();
        mCommands.pop_front();
    }
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool Combiner::Do()
{
    BOOST_FOREACH( wxCommand* command, mCommands )
    {
        command->Do();
    }
    return true;
}

bool Combiner::Undo()
{
    BOOST_REVERSE_FOREACH( wxCommand* command, mCommands )
    {
        command->Undo();
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Combiner::add(wxCommand* command)
{
    mCommands.push_back(command);
    mCommandName = command->GetName();
}

} // namespace