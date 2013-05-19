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
    //Fit();

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

    mTransitionOption->Show(isTransition);
    mVideoOption->Show(!isTransition);
    mAudioOption->Show(!isTransition);
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
}

void DetailsTrim::hide()
{
    requestShow(false);
}

}} // namespace