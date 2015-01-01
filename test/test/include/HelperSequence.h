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

#ifndef HELPER_SEQUENCE_H
#define HELPER_SEQUENCE_H

namespace test {

void ExtendSequenceWithRepeatedClips( model::SequencePtr sequence, model::IPaths files, int nRepeat, bool atBegin = false );

void ExtendSequenceWithRepeatedClipsAtBegin( model::SequencePtr sequence, model::IPaths files, int nRepeat );

void ExtendSequenceWithEmptyClipAtBegin( model::SequencePtr sequence, milliseconds time );

void ExtendSequenceWithStillImage( model::SequencePtr sequence );

void MakeSequenceEmpty( model::SequencePtr sequence );

void ExtendTrack(model::TrackPtr track, model::IPaths files, int nRepeat = 1);

void MakeTrackEmpty(model::TrackPtr track);

} // namespace

#endif