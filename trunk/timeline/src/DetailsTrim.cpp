#include "DetailsTrim.h"

#include "Convert.h"
#include "IClip.h"
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
    LOG_INFO;

    VAR_INFO(GetSize());

    addbox(_("Resizing video"));
    mVideo = new wxStaticText(this, wxID_ANY, "");
    mAudio = new wxStaticText(this, wxID_ANY, "");
    addoption(_("Video size"),mVideo);
    addoption(_("Audio size"),mAudio);

    // todo add 'adjust' info
    // todo handle if (event->getclip->isa<transition>()
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
    if (event.getValue().getActive())
    {
        TrimEvent& update = event.getValue();
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
        if (videoclip)
        {
            wxString s = wxString::Format("%s -> %s",
                model::Convert::ptsToHumanReadibleString(videoclip->getLength()),
                model::Convert::ptsToHumanReadibleString(videocliptrimmed?videocliptrimmed->getLength():0));
            mVideo->SetLabel(s);

        }
        if (audioclip)
        {
            wxString s = wxString::Format("%s -> %s",
                model::Convert::ptsToHumanReadibleString(audioclip->getLength()),
                model::Convert::ptsToHumanReadibleString(audiocliptrimmed?audiocliptrimmed->getLength():0));
            mAudio->SetLabel(s);
        }
        requestShow(true, _("Resizing ") + event.getValue().getClip()->getDescription());
    }
    else
    {
        requestShow(false);
    }
}

}} // namespace