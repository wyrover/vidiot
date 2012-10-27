#include "CommandProcessor.h"

#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CommandProcessor::CommandProcessor()
    :   wxCommandProcessor()
    ,   mUndoSize(0)
{
}

CommandProcessor::~CommandProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// wxCommandProcessor
//////////////////////////////////////////////////////////////////////////

bool CommandProcessor::Redo()
{
    mUndoSize++;
    ASSERT_MORE_THAN_ZERO(mUndoSize);
    return wxCommandProcessor::Redo();
}

bool CommandProcessor::Submit (wxCommand *command, bool storeIt)
{
    mUndoSize++;
    ASSERT_MORE_THAN_ZERO(mUndoSize);
    return wxCommandProcessor::Submit(command,storeIt);
}

bool CommandProcessor::Undo()
{
    mUndoSize++;
    ASSERT_MORE_THAN_EQUALS_ZERO(mUndoSize);
    return wxCommandProcessor::Undo();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int CommandProcessor::getUndoSize() const
{
    ASSERT_MORE_THAN_EQUALS_ZERO(mUndoSize);
    return mUndoSize;
}

} // namespace