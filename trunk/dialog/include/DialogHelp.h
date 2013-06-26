#ifndef DIALOG_HELP_H
#define DIALOG_HELP_H

namespace gui {

class DialogHelp
    :   public wxDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DialogHelp();
    virtual ~DialogHelp();

    void onBack(wxCommandEvent &event);

    wxHtmlWindow* mHtml;
    wxButton* mBack;
};

} // namespace

#endif // DIALOG_HELP_H