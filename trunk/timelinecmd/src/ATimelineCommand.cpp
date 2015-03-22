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

#include "ATimelineCommand.h"

#include "TimelinesView.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ATimelineCommand::ATimelineCommand(const model::SequencePtr& sequence)
    :   RootCommand()
    ,   mSequence(sequence)
{
}

ATimelineCommand::~ATimelineCommand()
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::SequencePtr ATimelineCommand::getSequence() const
{
    return mSequence;
}

gui::timeline::Timeline& ATimelineCommand::getTimeline() const
{
    return gui::TimelinesView::get().getTimeline(mSequence);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ATimelineCommand& obj)
{
    os << &obj << '|' << typeid(obj).name();
    return os;
}

}}} // namespace
