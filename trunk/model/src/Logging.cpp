// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
const bool ORDER_NORMAL = false;
const bool ORDER_REVERSED = true;

std::ostream& dump(std::ostream& os, const SequencePtr& sequence, int depth)
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
        dump(os, videotracks, ORDER_REVERSED, depth + 1); // Same order as in GUI
        os << tab << std::endl << "AUDIO:";
        dump(os, sequence->getAudioTracks(), ORDER_NORMAL, depth + 1);
    }
    return os;
}

std::ostream& dump(std::ostream& os, const TrackPtr& track, int depth)
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
        dump(os, track->getClips(), ORDER_NORMAL, depth + 1);
    }
    return os;
}

std::ostream& dump(std::ostream& os, const Tracks& tracks, int depth)
{
    dump(os, tracks, false, depth);
    return os;
}

std::ostream& dump(std::ostream& os, const IClipPtr& clip, int depth)
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

std::ostream& dump(std::ostream& os, const IClips& clips, int depth)
{
    dump(os, clips, false, depth);
    return os;
}

} // namespace