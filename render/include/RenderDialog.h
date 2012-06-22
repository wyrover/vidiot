#ifndef RENDER_DIALOG_H
#define RENDER_DIALOG_H

#include <wx/choice.h>
#include <wx/gauge.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/event.h>
#include <wx/bookctrl.h>
#include <wx/notebook.h>
#include <boost/shared_ptr.hpp>

namespace model { namespace render {
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

    explicit RenderDialog(model::render::RenderPtr render);

	virtual ~RenderDialog();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onFormatChoiceChanged(wxCommandEvent& event);
    void onFileButtonPressed(wxCommandEvent& event);

private:

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

    model::render::RenderPtr mRender;

    wxBookCtrlBase* mBook;

    wxChoice* mFormat;
    wxTextCtrl* mFile;
    wxButton* mFileButton;
    wxButton* mRenderButton;
    wxGauge* mProgress;

    wxPanel* mVideo;
    wxPanel* mAudio;

    //////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

    void enableCodecInfo();

};

} // namespace

#endif // RENDER_DIALOG_H