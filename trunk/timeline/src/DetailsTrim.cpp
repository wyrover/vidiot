#include "DetailsTrim.h"

#include "Convert.h"
#include "IClip.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "AudioClip.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static wxString sNotes(_("Notes"));

DetailsTrim::DetailsTrim(wxWindow* parent, Timeline& timeline)
    :   DetailsPanel(parent,timeline)
{
    VAR_DEBUG(this);

    addBox(_("Resizing"));
    mVideo = new wxStaticText(this, wxID_ANY, "");
    mAudio = new wxStaticText(this, wxID_ANY, "");
    mTransition = new wxStaticText(this, wxID_ANY, "");
    addOption(_("Video size"),mVideo);
    addOption(_("Audio size"),mAudio);
    addOption(_("Transition size"),mTransition);

    addBox(sNotes);
    mNote = new wxStaticText(this, wxID_ANY, _("Press SHIFT to remove blank areas during the trimming"));
    addOption(_("Shift"), mNote);

    GetSizer()->AddStretchSpacer();

    Fit();

    VAR_INFO(GetSize());
}

DetailsTrim::~DetailsTrim()
{
}

//////////////////////////////////////////////////////////////////////////
// TRIM UPDATES
//////////////////////////////////////////////////////////////////////////

void DetailsTrim::show( model::IClipPtr src, model::IClipPtr trg, model::IClipPtr srcLink, model::IClipPtr trgLink  )
{
    ASSERT_NONZERO(src);
    auto showLength = [] (model::IClipPtr clipold, model::IClipPtr clipnew, wxStaticText* textbox)
    {
        if (!clipold) { return; }
        wxString s = wxString::Format("%s -> %s",
            model::Convert::ptsToHumanReadibleString(clipold->getLength()),
            model::Convert::ptsToHumanReadibleString(clipnew?clipnew->getLength():0));
        textbox->SetLabel(s);
    };

    if (!requestsToBeShown())
    {
        requestShow(true, _("Resizing ") + src->getDescription());
        Fit();
    }

    bool isTransition = src->isA<model::Transition>();
    bool isVideo = src->isA<model::VideoClip>();

    showOption(mTransition,isTransition);
    showOption(mVideo,!isTransition);
    showOption(mAudio,!isTransition);
    showBox(sNotes,!isTransition);
    if (src->isA<model::Transition>())
    {
        showLength(src,trg,mTransition);
    }
    else
    {
        if (isVideo)
        {
            showLength(src,trg,mVideo);
            showLength(srcLink,trgLink,mAudio);
        }
        else
        {
            showLength(srcLink,trgLink,mVideo);
            showLength(src,trg,mAudio);
        }
    }
    Fit();
}

void DetailsTrim::hide()
{
    requestShow(false);
}

}} // namespace