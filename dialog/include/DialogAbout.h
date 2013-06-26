#ifndef DIALOG_ABOUT_H
#define DIALOG_ABOUT_H

namespace gui {

class DialogAbout
    :   public wxDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DialogAbout();
    virtual ~DialogAbout();

    void onBack(wxCommandEvent &event);

    wxHtmlWindow* mHtml;
    wxButton* mBack;
};

} // namespace

#endif // DIALOG_ABOUT_H