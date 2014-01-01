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

#ifndef SELECTION_H
#define SELECTION_H

#include "Part.h"
#include "PositionInfo.h"

namespace model {
    class EventRemoveClips;
}

namespace gui { namespace timeline {

class Selection
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Selection(Timeline* timeline);
    virtual ~Selection();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void updateOnLeftClick(const PointerPositionInfo& info);
    void updateOnRightClick(const PointerPositionInfo& info);

    /// \return true if no clip is selected
    bool isEmpty() const;

    /// Deletes all selected clips.
    void deleteClips(bool shift = false);

    /// Unselect all clips
    void unselectAll();

    /// Change the selection into the given list of clips.
    /// List may contain 0-ptrs, these are ignored.
    /// Any other selection is cleared.
    void change(model::IClips selection);


    /// Whenever the sequence is changed (by a replacement of clips) then the
    /// list of selected clips must be updated accordingly.
    void updateOnEdit();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mPreviouslyClicked;      ///< Clip which was previously (de)selected.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void selectClipAndLink(model::IClipPtr clip, bool selected);
    void selectClip(model::IClipPtr clip, bool selected);
    void setPreviouslyClicked(model::IClipPtr clip);
};

}} // namespace

#endif // SELECTION_H