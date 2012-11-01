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

const int TAB_WIDTH = 2;

std::ostream& dump(std::ostream& os, SequencePtr sequence, int depth)
{
    wxString tab(' ', depth * TAB_WIDTH);
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
        os << tab << std::endl << "AUDIO:";
        dump(os, sequence->getAudioTracks(), depth + 1);
    }
    return os;
}

std::ostream& dump(std::ostream& os, TrackPtr track, int depth)
{
    wxString tab(' ', depth * TAB_WIDTH);
    os << std::endl << tab;
    if (!track)
    {
        os << "0-TrackPtr";
    }
    else
    {
        os << (track->isA<VideoTrack>() ? "VideoTrack "  : "AudioTrack ") << track->getIndex() << ": " << (*track);
        dump(os, track->getClips(), depth + 1);
    }
    return os;
}

std::ostream& dump(std::ostream& os, IClipPtr clip, int depth)
{
    wxString tab(' ', depth * TAB_WIDTH);
    os << std::endl << tab;
    if (!clip)
    {
        os << "0-ClipPtr";
    }
    else
    {
        clip->dump(os);
    }
    return os;
}

} // namespace