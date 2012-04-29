#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <wx/cmdproc.h>

namespace model {

class CommandProcessor
    :   public wxCommandProcessor
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CommandProcessor();
    ~CommandProcessor();

    static CommandProcessor& get();
};

} // namespace

#endif // COMMAND_PROCESSOR_H