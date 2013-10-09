// Copyright 2013 Eric Raijmakers.
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

#ifndef HELPER_SEQUENCE_H
#define HELPER_SEQUENCE_H

#include <boost/shared_ptr.hpp>

namespace model {
class File;
typedef boost::shared_ptr<File> FilePtr;
typedef std::list< FilePtr > Files;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
}

namespace test {

void extendSequenceWithRepeatedClips( model::SequencePtr sequence, model::IPaths files, int nRepeat );

void extendSequenceWithStillImage( model::SequencePtr sequence );

} // namespace

#endif // HELPER_SEQUENCE_H