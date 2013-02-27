#ifndef DETAILS_CLIP_H
#define DETAILS_CLIP_H

#include "DetailsPanel.h"
#include "UtilEnumSelector.h"
#include "Enums.h"

namespace model {
    class ChangeVideoClipTransform;
    class EventChangeVideoClipOpacity;
    class EventChangeVideoClipScaling;
    class EventChangeVideoClipScalingFactor;
    class EventChangeVideoClipAlignment;
    class EventChangeVideoClipPosition;
    class EventChangeVideoClipMinPosition;
    class EventChangeVideoClipMaxPosition;
}

namespace gui { namespace timeline {
class EventSelectionUpdate;

class DetailsClip
:   public DetailsPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsClip(wxWindow* parent, Timeline& timeline);
    virtual ~DetailsClip();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getClip() const;
    void setClip(model::IClipPtr clip);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onShow(wxShowEvent& event);
    void onLengthButtonPressed(wxCommandEvent& event);
    void onOpacitySliderChanged(wxCommandEvent& event);
    void onOpacitySpinChanged(wxSpinEvent& event);
    void onScalingChoiceChanged(wxCommandEvent& event);
    void onScalingSliderChanged(wxCommandEvent& event);
    void onScalingSliderChanging(wxCommandEvent& event);
    void onScalingSpinChanged(wxSpinDoubleEvent& event);
    void onAlignmentChoiceChanged(wxCommandEvent& event);
    void onPositionXSliderChanged(wxCommandEvent& event);
    void onPositionXSpinChanged(wxSpinEvent& event);
    void onPositionYSliderChanged(wxCommandEvent& event);
    void onPositionYSpinChanged(wxSpinEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpacityChanged(model::EventChangeVideoClipOpacity& event);
    void onScalingChanged(model::EventChangeVideoClipScaling& event);
    void onScalingFactorChanged(model::EventChangeVideoClipScalingFactor& event);
    void onAlignmentChanged(model::EventChangeVideoClipAlignment& event);
    void onPositionChanged(model::EventChangeVideoClipPosition& event);
    void onMinPositionChanged(model::EventChangeVideoClipMinPosition& event);
    void onMaxPositionChanged(model::EventChangeVideoClipMaxPosition& event);

    //////////////////////////////////////////////////////////////////////////
    // SELECTION EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSelectionChanged( timeline::EventSelectionUpdate& event );

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    wxSlider* getOpacitySlider() const;
    wxSpinCtrl* getOpacitySpin() const;
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

    std::list<wxToggleButton*> mLengthButtons;

    wxSpinCtrl* mOpacitySpin;
    wxSlider* mOpacitySlider;

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
    void preview();

    /// When a slider or spin control is changed for one of the position values, then update
    /// the alignment choice accordingly.
    void updateAlignment(bool horizontalchange);

    void updateLengthButtons();
};
}} // namespace

#endif // DETAILS_CLIP_H