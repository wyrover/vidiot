// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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