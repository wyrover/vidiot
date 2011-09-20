#ifndef ROOT_COMMAND_H
#define ROOT_COMMAND_H

#include <wx/cmdproc.h>
#include <wx/intl.h>

namespace command {

class RootCommand : public wxCommand
{
public:
    RootCommand();
    virtual ~RootCommand();
    wxString GetName() const override;

protected:
    wxString mCommandName;
};

} // namespace

#endif // ROOT_COMMAND_H
