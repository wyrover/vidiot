#include "Logging.h"

#include "AudioClip.h"
#include "EmptyClip.h"
#include "Sequence.h"
#include "Track.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "VideoTransition.h"

namespace model {

std::ostream& dump(std::ostream& os, SequencePtr sequence, int depth)
{
    wxString tab(' ', depth * 4);
    os << std::endl << "Sequence:";
    if (!sequence)
    {
        os << tab << "0-SequencePtr";
    }
    else
    {
        os << tab << (*sequence);
        os << tab << std::endl << "VIDEO:";
        model::Tracks videotracks = sequence->getVideoTracks();
        videotracks.reverse(); // Same order as in GUI
        dump(os, videotracks, depth + 1);
        os << tab << std::endl << "AUDIO:" << std::endl;
        dump(os, sequence->getAudioTracks(), depth + 1);
    }
    return os;
}

std::ostream& dump(std::ostream& os, TrackPtr track, int depth)
{
    wxString tab(' ', depth * 4);
    os << std::endl << tab;
    if (!track)
    {
        os << "0-TrackPtr";
    }
    else if (track->isA<VideoTrack>())
    {
        os << "VideoTrack " << track->getIndex() << ": " << (*track);
        dump(os, track->getClips(), depth + 1);
    }
    else if (track->isA<AudioTrack>())
    {
        os << "AudioTrack " << track->getIndex() << ": " << (*track);
        dump(os, track->getClips(), depth + 1);
    }
    return os;
}

std::ostream& dump(std::ostream& os, IClipPtr clip, int depth)
{
    wxString tab(' ', depth * 4);
    os << std::endl << tab;
    if (!clip)
    {
        os << "0-ClipPtr";
    }
    else
    {
        os << std::setfill('0') << std::setw(2) << clip->getIndex() << ' ';
        if (clip->isA<VideoTransition>())
        {
            os << "VideoTransition: " << *(boost::dynamic_pointer_cast<model::VideoTransition>(clip));
        }
        else if (clip->isA<model::EmptyClip>())
        {
            os << "EmptyClip:       " << *(boost::dynamic_pointer_cast<model::EmptyClip>(clip));
        }
        else if (clip->isA<model::VideoClip>()) // todo this must be via inherit+overload
        {
            os << "VideoClip:       " << *(boost::dynamic_pointer_cast<model::VideoClip>(clip));
        }
        else if (clip->isA<model::AudioClip>()) // todo this must be via inherit+overload
        {
            os << "AudioClip:       " << *(boost::dynamic_pointer_cast<model::AudioClip>(clip));
        }
        else
        {
            FATAL("Clip type unknown.");
        }
    }
    return os;
}

} // namespace