#ifndef DETAILS_PANEL_H
#define DETAILS_PANEL_H

#include "Part.h"

namespace gui { namespace timeline {

class DetailsPanel
:   public wxPanel
,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsPanel(wxWindow* parent, Timeline& timeline);
    ~DetailsPanel();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool requestsToBeShown() const;
    wxString getTitle() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE TOWARDS ACTUAL DETAILS VIEWS
    //////////////////////////////////////////////////////////////////////////

    void requestShow(bool show, wxString title = "");

    void addBox(const wxString& name);

    void showBox(const wxString& name, bool show = true);

    void addOption(const wxString& name, wxWindow* widget);

    /// Show/hide an option and its title
    /// \param widget option that must be shown/hidden
    /// \param show if true then show, otherwise hide
    void showOption(wxWindow* widget, bool show = true);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mShow;
    wxString mTitle;

    wxBoxSizer*      mTopSizer;  ///< sizer for panel
    wxSizer*         mBoxSizer;  ///< sizer for current box

    std::map<wxString, wxSizer*> mBoxes;

    std::map<wxWindow*, wxSizer*> mMapWindowToSizer;
    std::map<wxWindow*, wxStaticText*> mMapWindowToTitle;

};

}} // namespace

#endif // DETAILS_PANEL_H