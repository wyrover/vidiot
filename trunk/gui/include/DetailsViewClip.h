#ifndef DETAILS_VIEW_CLIP_H
#define DETAILS_VIEW_CLIP_H

#include <wx/panel.h>
#include <wx/sizer.h>
#include <boost/shared_ptr.hpp>
#include "Enums.h"
#include "UtilEnumSelector.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
}

namespace gui {

class DetailsViewClip
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsViewClip(wxWindow* parent, model::IClipPtr clip);
    virtual ~DetailsViewClip();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip;

    wxBoxSizer*         mTopSizer;  ///< sizer for panel
    wxBoxSizer*         mBoxSizer;  ///< sizer for current box

    EnumSelector<model::VideoScaling>* mSelectScaling;
    EnumSelector<model::VideoAlignment>* mSelectAlignment;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
};

} // namespace

#endif // DETAILS_VIEW_CLIP_H