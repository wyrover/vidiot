#ifndef DETAILS_TRIM_H
#define DETAILS_TRIM_H

#include "DetailsPanel.h"

namespace gui { namespace timeline {

class EventTrimUpdate;

class DetailsTrim
:   public DetailsPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsTrim(wxWindow* parent, Timeline& timeline);
    virtual ~DetailsTrim();

    //////////////////////////////////////////////////////////////////////////
    // TRIM UPDATES
    //////////////////////////////////////////////////////////////////////////

    void show(model::IClipPtr src, model::IClipPtr trg, model::IClipPtr srcLink, model::IClipPtr trgLink);
    void hide();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxStaticText*       mTransition;
    wxStaticText*       mVideo;
    wxStaticText*       mAudio;

    wxStaticText*       mNote;
};

}} // namespace

#endif // DETAILS_TRIM_H