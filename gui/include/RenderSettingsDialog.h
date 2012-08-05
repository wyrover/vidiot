#ifndef RENDER_DIALOG_H
#define RENDER_DIALOG_H

#include "ICodecParameter.h"
#include "UtilEnumSelector.h"
#include "UtilInt.h"

namespace gui {

class RenderSettingsDialog
    :   public wxDialog
    ,   public model::render::ICodecParameterChangeListener
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit RenderSettingsDialog(model::SequencePtr sequence);
	virtual ~RenderSettingsDialog();
    static RenderSettingsDialog& get();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoCodecChanged(wxCommandEvent& event);
    void onAudioCodecChanged(wxCommandEvent& event);
    void onFileButtonPressed(wxCommandEvent& event);
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

    wxButton* mRenderButton;
    wxButton* mOkButton;
    wxButton* mCancelButton;
    wxButton* mApplyButton;
    wxButton* mSetDefaultButton;

    bool mRendering;

    pts mLength;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateAudioCodec();
    void updateVideoCodec();
    void changeAudioCodecInfo(model::render::AudioCodecPtr oldAudioCodec, model::render::AudioCodecPtr newAudioCodec);
    void changeVideoCodecInfo(model::render::VideoCodecPtr oldVideoCodec, model::render::VideoCodecPtr newVideoCodec);
    void enableRenderButton();
    void enableSetDefaultButton();
    bool checkFilename();
};

} // namespace

#endif // RENDER_DIALOG_H