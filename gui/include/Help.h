#ifndef HELP_H
#define HELP_H

namespace gui {

class Help
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit Help(wxWindow* parent);
    virtual ~Help();

    void onBack(wxCommandEvent &event);

    wxHtmlWindow* mHtml;
    wxButton* mBack;
};

} // namespace

#endif // HELP_H