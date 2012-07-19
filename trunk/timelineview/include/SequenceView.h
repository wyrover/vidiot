#ifndef SEQUENCE_VIEW_H
#define SEQUENCE_VIEW_H

#include "View.h"

namespace model {
    class EventAddVideoTracks;
}

namespace gui { namespace timeline {

struct PointerPositionInfo;
class TimescaleView;
class IntervalsView;

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
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    TimescaleView& getTimescale();
    const TimescaleView& getTimescale() const;

    VideoView& getVideo();
    const VideoView& getVideo() const;

    AudioView& getAudio();
    const AudioView& getAudio() const;

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
    IntervalsView* mIntervalsView;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // SEQUENCE_VIEW_H