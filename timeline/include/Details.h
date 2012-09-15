#ifndef DETAILS_H
#define DETAILS_H

#include "Part.h"

namespace gui { namespace timeline {

class IDetails;

class Details
    :   public wxPanel
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Details(wxWindow* parent, Timeline* timeline);
    virtual ~Details();

    void update();

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    wxWindow* getCurrent() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxStaticText* mHeader;
    wxWindow* mCurrent;
    std::list<IDetails*> mDetails;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

};

class IDetails
:   public wxPanel
,   public Part
{
public:
    explicit IDetails(Details* parent, Timeline& timeline)
        :   wxPanel(parent)
        ,   Part(&timeline)
        ,   mShow(false)
        ,   mTitle("")
    {
    }
    bool requestsToBeShown() const
    {
        return mShow;
    }
    wxString getTitle() const
    {
        return mTitle;
    }
protected:
    void requestShow(bool show, wxString title = "")
    {
        mShow = show;
        mTitle = title;
        static_cast<Details*>(GetParent())->update();
    }
private:
    bool mShow;
    wxString mTitle;
};

}} // namespace

#endif // DETAILS_H