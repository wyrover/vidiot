// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef MODEL_DEBUG_H
#define MODEL_DEBUG_H

namespace model {

template <class OBJ>
std::ostream& dump(std::ostream& os, const std::vector< boost::shared_ptr< OBJ > >& list, bool reverse, int depth = 0 )
{
    typedef boost::shared_ptr< OBJ > OBJPtr;
    if (reverse)
    {
        for (OBJPtr obj : boost::adaptors::reverse(list))
        {
            dump(os, obj, depth);
        }
    }
    else
    {
        for (OBJPtr obj : list)
        {
            dump(os, obj, depth);
        }
    }
    return os;
}

std::ostream& dump(std::ostream& os, const SequencePtr& sequence, int depth = 0);
std::ostream& dump(std::ostream& os, const Tracks& tracks, int depth = 0);
std::ostream& dump(std::ostream& os, const TrackPtr& track, int depth = 0);
std::ostream& dump(std::ostream& os, const IClips& clips, int depth = 0);
std::ostream& dump(std::ostream& os, const IClipPtr& clip, int depth = 0);

template <class OBJ>
std::string dump(const OBJ& o, int depth = 0)
{
    std::ostringstream os;
    dump(os,o,depth);
    return os.str();
}

#define DUMP(o) std::endl << #o << model::dump(o,1)

} // namespace

#endif
