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

#ifndef AUDIO_VIEW_H
#define AUDIO_VIEW_H

#include "View.h"

namespace model {
    class EventAddAudioTracks;
    class EventRemoveAudioTracks;
}

namespace gui { namespace timeline {
struct PointerPositionInfo;

class AudioView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    AudioView(View* parent);
    virtual ~AudioView();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void canvasResized(); ///< Must be called when the widget is resized

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

    /// \return y position of the track within this View
    pixel getPosition(model::TrackPtr track) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onAudioTracksAdded( model::EventAddAudioTracks& event );
    void onAudioTracksRemoved( model::EventRemoveAudioTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()

};

}} // namespace

#endif // AUDIO_VIEW_H