#ifndef DETAILS_CLIP_H
#define DETAILS_CLIP_H

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <boost/shared_ptr.hpp>
#include "VideoClipEvent.h"
#include "UtilEnumSelector.h"
#include "Part.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
class AudioClip;
typedef boost::shared_ptr<AudioClip> AudioClipPtr;
class ChangeVideoClipTransform;
}

namespace gui { namespace timeline {

class DetailsClip
:   public wxPanel
,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsClip(wxWindow* parent, Timeline& timeline, model::IClipPtr clip);
    virtual ~DetailsClip();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onScalingChoiceChanged(wxCommandEvent& event);
    void onScalingSliderChanged(wxCommandEvent& event);
    void onScalingSpinChanged(wxSpinDoubleEvent& event);
    void onAlignmentChoiceChanged(wxCommandEvent& event);
    void onPositionXSliderChanged(wxCommandEvent& event);
    void onPositionXSpinChanged(wxSpinEvent& event);
    void onPositionYSliderChanged(wxCommandEvent& event);
    void onPositionYSpinChanged(wxSpinEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onScalingChanged(model::EventChangeVideoClipScaling& event);
    void onScalingDigitsChanged(model::EventChangeVideoClipScalingDigits& event);
    void onAlignmentChanged(model::EventChangeVideoClipAlignment& event);
    void onPositionChanged(model::EventChangeVideoClipPosition& event);
    void onMinPositionChanged(model::EventChangeVideoClipMinPosition& event);
    void onMaxPositionChanged(model::EventChangeVideoClipMaxPosition& event);

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    EnumSelector<model::VideoScaling>* getScalingSelector() const;
    wxSlider* getScalingSlider() const;
    wxSpinCtrlDouble* getScalingSpin() const;
    EnumSelector<model::VideoAlignment>* getAlignmentSelector() const;
    wxSlider* getPositionXSlider() const;
    wxSpinCtrl* getPositionXSpin() const;
    wxSlider* getPositionYSlider() const;
    wxSpinCtrl* getPositionYSpin() const;

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

    wxSpinCtrl* mPositionXSpin;
    wxSlider* mPositionXSlider;
    wxSpinCtrl* mPositionYSpin;
    wxSlider* mPositionYSlider;

    model::ChangeVideoClipTransform* mCommand;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void makeCommand();
    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
    void preview();

    /// When a slider or spin control is changed for one of the position values, then update
    /// the alignment choice accordingly.
    void updateAlignment(bool horizontalchange);

};

}} // namespace

#endif // DETAILS_CLIP_H