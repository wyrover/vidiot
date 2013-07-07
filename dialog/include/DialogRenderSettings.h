#ifndef DIALOG_RENDER_SETTINGS_H
#define DIALOG_RENDER_SETTINGS_H

#include "ICodecParameter.h"
#include "UtilEnumSelector.h"
#include "UtilInt.h"
#include "UtilSingleInstance.h"

namespace gui {

class DialogRenderSettings
    :   public wxDialog
    ,   public model::render::ICodecParameterChangeListener
    ,   public SingleInstance<DialogRenderSettings>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DialogRenderSettings(model::SequencePtr sequence);
    virtual ~DialogRenderSettings();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoCodecChanged(wxCommandEvent& event);
    void onAudioCodecChanged(wxCommandEvent& event);
    void onFileButtonPressed(wxCommandEvent& event);
    void onRenderSeparationChanged(wxCommandEvent& event);
    void onRenderButtonPressed(wxCommandEvent& event);
    void onOkButtonPressed(wxCommandEvent& event);
    void onCancelButtonPressed(wxCommandEvent& event);
    void onApplyButtonPressed(wxCommandEvent& event);
    void onSetDefaultButtonPressed(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    //  model::render::ICodecParameterChangeListener interface
    //////////////////////////////////////////////////////////////////////////

    void onParameterChange() override;

    //////////////////////////////////////////////////////////////////////////
    // TESTS
    //////////////////////////////////////////////////////////////////////////

    wxButton* getFileButton() const;
    EnumSelector<int>* getVideoCodecButton() const;
    wxCheckBox* getRenderSeparationCheckBox() const;
    wxButton* getRenderButton() const;
    wxButton* getOkButton() const;
    wxButton* getCancelButton() const;
    wxButton* getApplyButton() const;
    wxWindow* getAudioParam(int index) const;
    wxWindow* getVideoParam(int index) const;

private:

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    model::render::RenderPtr mOriginal;
    model::render::RenderPtr mNew;

    wxTextCtrl* mFile;
    wxButton* mFileButton;
    EnumSelector<int>* mVideoCodec;
    EnumSelector<int>* mAudioCodec;
    wxScrolledWindow* mVideoParameters;
    wxScrolledWindow* mAudioParameters;

    wxCheckBox* mRenderSeparation;

    wxButton* mRenderButton;
    wxButton* mOkButton;
    wxButton* mCancelButton;
    wxButton* mApplyButton;
    wxButton* mSetDefaultButton;

    bool mRendering;

    pts mLength;

    std::vector<wxWindow*> mAudioParameterWidgets;
    std::vector<wxWindow*> mVideoParameterWidgets;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateAudioCodec();
    void updateVideoCodec();
    void changeAudioCodecInfo(model::render::AudioCodecPtr oldAudioCodec, model::render::AudioCodecPtr newAudioCodec);
    void changeVideoCodecInfo(model::render::VideoCodecPtr oldVideoCodec, model::render::VideoCodecPtr newVideoCodec);
    void enableRenderButton();
    void enableSetDefaultButton();
    bool check();
    void applyNewRender();
};

} // namespace

#endif // DIALOG_RENDER_SETTINGS_H