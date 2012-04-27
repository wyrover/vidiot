#ifndef ROOT_COMMAND_H
#define ROOT_COMMAND_H

#include <wx/cmdproc.h>
#include <wx/intl.h>

namespace command {

class RootCommand : public wxCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    RootCommand();
    virtual ~RootCommand();

    //////////////////////////////////////////////////////////////////////////
    // SUBMIT
    //////////////////////////////////////////////////////////////////////////

    void Submit();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxString GetName() const override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mCommandName;
};

} // namespace

#endif // ROOT_COMMAND_H