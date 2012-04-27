#ifndef DETAILS_VIEW_CLIP_H
#define DETAILS_VIEW_CLIP_H

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <boost/shared_ptr.hpp>
#include "VideoClipEvent.h"
#include "UtilEnumSelector.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
class AudioClip;
typedef boost::shared_ptr<AudioClip> AudioClipPtr;
class ChangeVideoClipTransform;
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
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onScalingChoiceChanged(wxCommandEvent& event);
    void onScalingSliderChanged(wxCommandEvent& event);
    void onScalingSpinChanged(wxSpinDoubleEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onScalingChanged(model::EventChangeVideoClipScaling& event);
    void onScalingFactorChanged(model::EventChangeVideoClipScalingFactor& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr     mClip;
    model::VideoClipPtr mVideoClip;
    model::AudioClipPtr mAudioClip;

    wxBoxSizer*         mTopSizer;  ///< sizer for panel
    wxBoxSizer*         mBoxSizer;  ///< sizer for current box

    EnumSelector<model::VideoScaling>* mSelectScaling;

    wxSpinCtrlDouble* mScalingSpin;
    wxSlider* mScalingSlider;

    EnumSelector<model::VideoAlignment>* mSelectAlignment;

    wxSpinCtrl* mOffsetXSpin;
    wxSlider* mOffsetXSlider;
    wxSpinCtrl* mOffsetYSpin;
    wxSlider* mOffsetYSlider;

    model::ChangeVideoClipTransform* mCommand;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
};
} // namespace

#endif // DETAILS_VIEW_CLIP_H