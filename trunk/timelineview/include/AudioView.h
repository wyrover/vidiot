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
    class EventHeightChanged;
}

namespace gui { namespace timeline {
struct PointerPositionInfo;
class DividerView;

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
    // VIEW
    //////////////////////////////////////////////////////////////////////////

    pixel getX() const override;
    pixel getY() const override;
    pixel getW() const override;
    pixel getH() const override;

    void invalidateRect() override;

    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    mutable boost::optional<pixel> mHeight; ///< Can be reset to ensure recalc.

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onAudioTracksAdded( model::EventAddAudioTracks& event );
    void onAudioTracksRemoved( model::EventRemoveAudioTracks& event );
    void onTrackHeightChanged( model::EventHeightChanged& event );
};

}} // namespace

#endif // AUDIO_VIEW_H