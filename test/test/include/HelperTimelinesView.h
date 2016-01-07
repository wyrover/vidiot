// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

namespace gui { namespace timeline {
class Timeline;
}}

namespace test {

/// \return Currently active seqence (the sequence for which the timeline is the active notebook page)
model::SequencePtr getSequence();

/// \return Clone of Render associated with the currently opened sequence
model::render::RenderPtr GetCurrentRenderSettings();

/// Return the opened timeline for a sequence
/// \param sequence if this equals 0 then the active timeline is returned
gui::timeline::Timeline& getTimeline(model::SequencePtr sequence = model::SequencePtr());

} // namespace
