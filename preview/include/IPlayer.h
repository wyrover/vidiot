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

struct ResumeInfo
{
    bool playing = false;
    // todo add resume position here
    boost::optional< std::pair<pts, pts> > range = boost::none;
};

struct IPlayer        // todo separate file and move all player related stuff into sep folder   (preview)
{
    virtual ResumeInfo pause() = 0;
    virtual void resume(const ResumeInfo& info) = 0;
};

