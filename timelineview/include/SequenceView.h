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

#include "View.h"

namespace model {
    class EventHeightChanged;
    class EventLengthChanged;
}

namespace gui { namespace timeline {

class DividerView;
class TimescaleView;
struct PointerPositionInfo;

class SequenceView
    :   public View
{
public:

    static const pixel MinimalGreyAboveVideoTracksHeight;
    static const pixel MinimalGreyBelowAudioTracksHeight;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    SequenceView(Timeline* timeline);
    virtual ~SequenceView();

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
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSequenceHeightChanged(model::EventHeightChanged& event);
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

    void setDividerPosition(pixel position);
    void resetDividerPosition();

    void getPositionInfo(const wxPoint& position, PointerPositionInfo& info ) const;

    pts getDefaultLength() const;
    void setMinimumLength(pts length);

    wxSize getDefaultSize() const;

    /// Set this to avoid scheduling clip previews on a separate thread.
    /// During some operations, it's much more intuitive to have real-time
    /// feedback, even in the timeline view. Examples are trimming (show proper
    /// 'new' thumbnail) and moving audio key frame positions (update peak
    /// view immediately).
    inline void setRealtimeRedrawing(bool enable) { mRealtime = enable; };
    inline bool getRealtimeRedrawing() const { return mRealtime; }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    TimescaleView* mTimescaleView;
    VideoView* mVideoView;
    DividerView* mDividerView;
    AudioView*  mAudioView;
    pts mMinimumLength;
    bool mRealtime = false;
};

}} // namespace
