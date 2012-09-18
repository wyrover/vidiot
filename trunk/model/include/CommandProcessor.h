#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "UtilSingleInstance.h"

namespace model {

class CommandProcessor
    :   public wxCommandProcessor
    ,   public SingleInstance<CommandProcessor>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CommandProcessor();
    ~CommandProcessor();
};

} // namespace

#endif // COMMAND_PROCESSOR_H