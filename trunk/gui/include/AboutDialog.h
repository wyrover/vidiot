#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <wx/html/htmlwin.h>

namespace gui {

class AboutDialog
    :   public wxDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit AboutDialog();
    virtual ~AboutDialog();

    void onBack(wxCommandEvent &event);

    wxHtmlWindow* mHtml;
    wxButton* mBack;
};

} // namespace

#endif // ABOUT_DIALOG_H