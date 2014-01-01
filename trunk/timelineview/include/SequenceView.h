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

#ifndef SEQUENCE_VIEW_H
#define SEQUENCE_VIEW_H

#include "View.h"

namespace model {
    class EventLengthChanged;
}

namespace gui { namespace timeline {

struct PointerPositionInfo;
class TimescaleView;

class SequenceView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    SequenceView(View* parent);
    virtual ~SequenceView();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

     void onSequenceLengthChanged(model::EventLengthChanged& event);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    TimescaleView& getTimescale();
    const TimescaleView& getTimescale() const;

    VideoView& getVideo();
    const VideoView& getVideo() const;

    AudioView& getAudio();
    const AudioView& getAudio() const;

    void canvasResized(); ///< Must be called when the widget is resized

    pixel minimumWidth() const; ///< Required to avoid infinite recursion in SequenceView::requiredSize() and Video/AudioView::getSize()
    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    void setDividerPosition(pixel position);
    void resetDividerPosition();

    pixel getAudioPosition() const;
    pixel getVideoPosition() const;

    void getPositionInfo(wxPoint position, PointerPositionInfo& info ) const;

    /// \return y position (top) of given track (excluding the divider)
    pixel getPosition(model::TrackPtr track) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    TimescaleView* mTimescaleView;
    VideoView*  mVideoView;
    AudioView*  mAudioView;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // SEQUENCE_VIEW_H