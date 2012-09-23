#include "DetailsTrim.h"

#include "Convert.h"
#include "IClip.h"
#include "Transition.h"
#include "Trim.h"
#include "TrimEvent.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "AudioClip.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DetailsTrim::DetailsTrim(wxWindow* parent, Timeline& timeline)
    :   DetailsPanel(parent,timeline)
{
    VAR_DEBUG(this);

    addbox(_("Resizing"));
    mVideo = new wxStaticText(this, wxID_ANY, "");
    mAudio = new wxStaticText(this, wxID_ANY, "");
    mTransition = new wxStaticText(this, wxID_ANY, "");
    mVideoOption = addoption(_("Video size"),mVideo);
    mAudioOption = addoption(_("Audio size"),mAudio);
    mTransitionOption = addoption(_("Transition size"),mTransition);

    addbox(_("Notes"));
    wxStaticText* statictext = new wxStaticText(this, wxID_ANY, _("Press SHIFT to remove blank areas during the trimming"));
    addoption(_("Shift"), statictext);

    GetSizer()->AddStretchSpacer();
    Fit();

    getTrim().Bind(EVENT_TRIM_UPDATE, &DetailsTrim::onTrimChanged, this);
    VAR_INFO(GetSize());
}

DetailsTrim::~DetailsTrim()
{
    getTrim().Unbind(EVENT_TRIM_UPDATE, &DetailsTrim::onTrimChanged, this);
}

//////////////////////////////////////////////////////////////////////////
// TRIM EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsTrim::onTrimChanged( timeline::EventTrimUpdate& event )
{
    event.Skip();
    auto showLength = [] (model::IClipPtr clipold, model::IClipPtr clipnew, wxStaticText* textbox)
    {
        if (!clipold) { return; }
        wxString s = wxString::Format("%s -> %s",
            model::Convert::ptsToHumanReadibleString(clipold->getLength()),
            model::Convert::ptsToHumanReadibleString(clipnew?clipnew->getLength():0));
        textbox->SetLabel(s);
    };

    switch (event.getValue().getState())
    {
    case OperationStateStart:
        {
            requestShow(true, _("Resizing ") + event.getValue().getClip()->getDescription());
            Fit();
            break;
        }
    case OperationStateStop:
        {
            requestShow(false);
            break;
        }
    case OperationStateUpdate:
        {
            TrimEvent& update = event.getValue();

            if (update.getClip()->isA<model::Transition>())
            {
                mTransitionOption->Show(true);
                mVideoOption->Show(false);
                mAudioOption->Show(false);
                model::IClipPtr transition = update.getClip();
                model::IClipPtr transitiontrimmed = update.getClipTrimmed();
                showLength(transition,transitiontrimmed,mTransition);
            }
            else
            {
                mTransitionOption->Show(false);
                mVideoOption->Show(true);
                mAudioOption->Show(true);
                model::IClipPtr videoclip;
                model::IClipPtr audioclip;
                model::IClipPtr videocliptrimmed;
                model::IClipPtr audiocliptrimmed;
                if (update.getClip()->isA<model::VideoClip>())
                {
                    videoclip = update.getClip();
                    videocliptrimmed = update.getClipTrimmed();
                    audioclip = update.getLink();
                    audiocliptrimmed = update.getLinkTrimmed();
                }
                else
                {
                    audioclip = update.getClip();
                    audiocliptrimmed = update.getClipTrimmed();
                    videoclip = update.getLink();
                    videocliptrimmed = update.getLinkTrimmed();
                }
                showLength(videoclip,videocliptrimmed,mVideo);
                showLength(audioclip,audiocliptrimmed,mAudio);
            }
            break;
        }
    default:
        {
            FATAL("Unknown OperationState");
            break;
        }
    }
}

}} // namespace