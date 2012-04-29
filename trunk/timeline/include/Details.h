#ifndef DETAILS_H
#define DETAILS_H

#include <list>
#include <wx/panel.h>
#include "Part.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef std::list<IClipPtr> IClips;
}

namespace gui { namespace timeline {

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

    void onHide();

    //////////////////////////////////////////////////////////////////////////
    // SET THE CURRENTLY FOCUSED ITEM
    //////////////////////////////////////////////////////////////////////////

    void focus(model::IClipPtr clip);
    void focus(model::IClips clips);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void reset(wxString title, wxWindow* details);

};

}} // namespace

#endif // DETAILS_H