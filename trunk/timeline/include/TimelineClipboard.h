// Copyright 2014 Eric Raijmakers.
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

#ifndef TIMELINE_CLIPBOARD_H
#define TIMELINE_CLIPBOARD_H

#include "Part.h"

namespace gui { namespace timeline {

class TimelineClipboard
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TimelineClipboard(Timeline* timeline);
    virtual ~TimelineClipboard();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onCut();
    void onCopy();
    void onPaste();

private:

    //////////////////////////////////////////////////////////////////////////
    // MAIN WINDOW EDIT MENU
    //////////////////////////////////////////////////////////////////////////

    void onCutFromMainMenu(wxCommandEvent& event);
    void onCopyFromMainMenu(wxCommandEvent& event);
    void onPasteFromMainMenu(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    bool hasKeyboardFocus() const;
};

}} // namespace

#endif
