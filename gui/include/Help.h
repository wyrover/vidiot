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

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onLink( wxHtmlLinkEvent& event);
    void onBack(wxCommandEvent &event);
    void onHome(wxCommandEvent &event);
    void onForward(wxCommandEvent &event);
    void onTimer(wxTimerEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxHtmlWindow* mHtml;
    wxButton* mBack;
    wxButton* mHome;
    wxButton* mForward;

    wxTimer mHighlightTimer;
    boost::optional<wxRect> mHighlight;
    int mHighlightCount;

    //////////////////////////////////////////////////////////////////////////
    // HELPER CLASSES
    //////////////////////////////////////////////////////////////////////////

    struct HtmlWindow : public wxHtmlWindow
    {
        HtmlWindow(Help *owner)
            :   wxHtmlWindow(owner)
        {
        }
        virtual bool LoadPage(const wxString& location) override
        {
            bool result =  wxHtmlWindow::LoadPage(location);
            static_cast<Help*>(GetParent())->updateButtons();
            return result;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void home();
    void updateButtons();

};

} // namespace

#endif // HELP_H