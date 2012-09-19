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
    // TRIM EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onTrimChanged( timeline::EventTrimUpdate& event );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxSizer*            mTransitionOption;
    wxSizer*            mVideoOption;
    wxSizer*            mAudioOption;

    wxStaticText*       mTransition;
    wxStaticText*       mVideo;
    wxStaticText*       mAudio;
};

}} // namespace

#endif // DETAILS_TRIM_H