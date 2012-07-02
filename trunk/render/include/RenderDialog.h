#ifndef RENDER_DIALOG_H
#define RENDER_DIALOG_H

#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
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
    class EventRenderProgress;
    class EventRenderActive;
    class Render;
    typedef boost::shared_ptr<Render> RenderPtr;
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

    void onRenderProgress(model::render::EventRenderProgress& event);
    void onRenderActive(model::render::EventRenderActive& event);

    void onVideoCodecChanged(wxCommandEvent& event);
    void onAudioCodecChanged(wxCommandEvent& event);
    void onFileButtonPressed(wxCommandEvent& event);
    void onRenderButtonPressed(wxCommandEvent& event);
    void onSetDefaultButtonPressed(wxCommandEvent& event);
    void onFileNameEntered(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // TESTS
    //////////////////////////////////////////////////////////////////////////

    wxButton* getFileButton() const;

private:

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;

    wxBookCtrlBase* mBook;

    wxTextCtrl* mFile;
    wxButton* mFileButton;

    EnumSelector<int>* mVideoCodec;
    EnumSelector<int>* mAudioCodec;
    wxButton* mRenderButton;
    wxButton* mSetDefaultButton;
    wxStaticText* mProgressText;
    wxGauge* mProgress;

    wxPanel* mVideo;
    wxPanel* mAudio;

    bool mRendering;

    pts mLength;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateOnCodecChange();
    void enableCodecInfo();
    void enableRenderButton();
    bool checkFileName(wxFileName filename) const;

    model::render::RenderPtr getRender();

};

} // namespace

#endif // RENDER_DIALOG_H