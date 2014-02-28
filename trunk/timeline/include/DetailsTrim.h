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

#ifndef DETAILS_TRIM_H
#define DETAILS_TRIM_H

#include "DetailsPanel.h"

namespace gui { namespace timeline {

class EventTrimUpdate;

class DetailsTrim
:   public DetailsPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsTrim(wxWindow* parent, Timeline& timeline);
    virtual ~DetailsTrim();

    //////////////////////////////////////////////////////////////////////////
    // TRIM UPDATES
    //////////////////////////////////////////////////////////////////////////

    void show(const model::IClipPtr& src, const model::IClipPtr& trg, const model::IClipPtr& srcLink, const model::IClipPtr& trgLink);
    void hide();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxStaticText*       mTransition;
    wxStaticText*       mVideo;
    wxStaticText*       mAudio;

    wxStaticText*       mNote;
};

}} // namespace

#endif
