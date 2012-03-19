#include "Dump.h"

#include <boost/foreach.hpp>
#include "AudioClip.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "MousePointer.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "UtilLogWxWidgets.h"
#include "VideoClip.h"
#include "VideoTransition.h"
#include "VideoView.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Dump::Dump(Timeline* timeline)
:   Part(timeline)
{
    VAR_DEBUG(this);
}

Dump::~Dump()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// DEBUG
//////////////////////////////////////////////////////////////////////////

void Dump::dump()
{
    model::SequencePtr sequence = getSequence();
    wxString tab("    ");
    LOG_DEBUG << "============================================================";
    LOG_DEBUG << "LEFTDOWN:  " << getMousePointer().getLeftDownPosition();
    LOG_DEBUG << "RIGHTDOWN: " << getMousePointer().getRightDownPosition();
    LOG_DEBUG << "============================================================";
    VAR_DEBUG(*sequence);
    int tracknum = 0;
    BOOST_FOREACH( model::TrackPtr track, sequence->getVideoTracks() )
    {
        LOG_DEBUG
            << "-------------------- VIDEOTRACK " << tracknum++
            << " (length=" << track->getLength()
            << ", position=" << getSequenceView().getVideoPosition() + getSequenceView().getVideo().getPosition(track)
            << ") --------------------";
        LOG_DEBUG << tab << "TRACK " << *track;
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (clip->isA<model::VideoTransition>())
            {
                LOG_DEBUG << tab << tab << "TRANSITION " << *(boost::dynamic_pointer_cast<model::VideoTransition>(clip));
            }
            else if (clip->isA<model::EmptyClip>())
            {
                LOG_DEBUG << tab << tab << "EMPTY      " << *(boost::dynamic_pointer_cast<model::EmptyClip>(clip));
            }
            else
            {
                LOG_DEBUG << tab << tab << "CLIP       " << *(boost::dynamic_pointer_cast<model::VideoClip>(clip));
            }
        }
    }
    tracknum = 0;
    BOOST_FOREACH( model::TrackPtr track, sequence->getAudioTracks() )
    {
        LOG_DEBUG << "-------------------- AUDIOTRACK " << tracknum++ << " (length=" << track->getLength() << ") --------------------";
        LOG_DEBUG << tab << *track;
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            //if (clip->isA<model::AudioTransition>())
            //{
            //    LOG_DEBUG << tab << tab << "TRANSITION " << *(boost::dynamic_pointer_cast<model::AudioTransition>(clip));
            //}
            //else
            if (clip->isA<model::EmptyClip>())
            {
                LOG_DEBUG << tab << tab << "EMPTY      " << *(boost::dynamic_pointer_cast<model::EmptyClip>(clip));
            }
            else
            {
                LOG_DEBUG << tab << tab << "CLIP       " << *(boost::dynamic_pointer_cast<model::AudioClip>(clip));
            }
        }
    }
    LOG_DEBUG << "============================================================";
}

}} // namespace