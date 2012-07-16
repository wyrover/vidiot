#ifndef RENDER_DIALOG_H
#define RENDER_DIALOG_H

#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/gauge.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <boost/shared_ptr.hpp>
#include "UtilEnumSelector.h"
#include "UtilInt.h"

namespace model {
    class Sequence;
    typedef boost::shared_ptr<Sequence> SequencePtr;

    namespace render {
    class EventRenderActive;
    class Render;
    typedef boost::shared_ptr<Render> RenderPtr;
    class AudioCodec;
    typedef boost::shared_ptr<AudioCodec> AudioCodecPtr;
    class VideoCodec;
    typedef boost::shared_ptr<VideoCodec> VideoCodecPtr;
}}

namespace gui {

class RenderDialog
    :   public wxDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit RenderDialog(model::SequencePtr sequence);
	virtual ~RenderDialog();
    static RenderDialog& get();

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
};

} // namespace

#endif // RENDER_DIALOG_H