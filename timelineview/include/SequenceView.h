#ifndef SEQUENCE_VIEW_H
#define SEQUENCE_VIEW_H

#include "View.h"

namespace model { 
    class EventAddVideoTracks; 
}

namespace gui { namespace timeline {

struct PointerPositionInfo;

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

    VideoView& getVideo();
    const VideoView& getVideo() const;

    AudioView& getAudio();
    const AudioView& getAudio() const;

    pixel requiredWidth() const;  ///< @see View::requiredWidth()
    pixel requiredHeight() const; ///< @see View::requiredHeight()

    void setDividerPosition(pixel position);
    void resetDividerPosition();

    pixel getAudioPosition() const;
    pixel getVideoPosition() const;

    void getPositionInfo(wxPoint position, PointerPositionInfo& info ) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoView*  mVideoView;
    AudioView*  mAudioView;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const; ///< @see View::draw()
};

}} // namespace

#endif // SEQUENCE_VIEW_H
