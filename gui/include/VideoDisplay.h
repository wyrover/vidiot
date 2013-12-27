// Copyright 2013 Eric Raijmakers.
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

#ifndef GUI_VIDEO_DISPLAY_H
#define GUI_VIDEO_DISPLAY_H

#include <SoundTouch.h>
#include "VideoFrame.h"
#include "AudioChunk.h"

namespace gui {

class VideoDisplay
    :   public wxControl
{
public:

    static const int sMinimumSpeed;
    static const int sMaximumSpeed;
    static const int sDefaultSpeed;
    static const int sVideoFrameRate;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    VideoDisplay(wxWindow *parent, model::SequencePtr sequence);
    virtual ~VideoDisplay();

    //////////////////////////////////////////////////////////////////////////
    // CONTROL METHODS
    //////////////////////////////////////////////////////////////////////////

    void play();
    void stop();
    void moveTo(pts position);
    void setSpeed(int speed);
    int getSpeed() const;
    bool isPlaying() const;
    model::SequencePtr getSequence() const;

    //////////////////////////////////////////////////////////////////////////
    // AUDIO
    //////////////////////////////////////////////////////////////////////////

    /// Called when portaudio needs more audio.
    /// \param buffer target buffer to be filled
    /// \param frames number of frames to be output.
    /// \param playtime time at which this audio buffer will be played
    /// \return true if more data is available, false if no more data is available
    /// A frame is a combination of samples, one sample for each output channel.
    /// Thus, a stereo frame contains a left and a right sample.
    /// Method is public since it is called by the C callback.
    bool audioRequested(void *buffer, unsigned long frames, double playtime);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    /// The Renderer responsible for producing video and audio data.
    model::SequencePtr mSequence;

    /// Current playing speed
    int mSpeed;

    /// If set, indicates that video and audio buffers must end.
    bool mAbortThreads;

    /// true if there is currently a sequence being played.
    bool mPlaying;

    /// Holds the time at which the first audio buffer will be played.
    /// Effectively, that's the start time of the playback. Time is in
    /// milliseconds.
    int mStartTime;

    /// Holds the pts at which the playback was started (thus, the 0-point timewise)
    int mStartPts;

    /// Current time is updated on each new video frame. This time is in milliseconds.
    int mCurrentTime;

    /// This mutex guards setting mStartTime in the audio playing thread
    /// and reading it in the video display thread.
    boost::mutex mMutexPlaybackStarted;

    /// This condition signals the start of the playback. The playback is
    /// started when the first audio buffer is requested and the start time is set.
    boost::condition_variable conditionPlaybackStarted;

    //////////////////////////////////////////////////////////////////////////
    // AUDIO
    //////////////////////////////////////////////////////////////////////////

    int mNumberOfAudioChannels; ///< Number of audio channels to use for playback
    int mAudioSampleRate; ///< Audio frame rate to use for playback

    model::FifoAudio mAudioChunks;
    model::AudioChunkPtr mCurrentAudioChunk;

    boost::scoped_ptr<boost::thread> mAudioBufferThreadPtr;
    void audioBufferThread();

    /// Required for portaudio
    void* mAudioOutputStream;

    soundtouch::SoundTouch mSoundTouch;

    //////////////////////////////////////////////////////////////////////////
    // VIDEO
    //////////////////////////////////////////////////////////////////////////

    model::FifoVideo mVideoFrames;
    model::VideoFramePtr mCurrentVideoFrame;
    boost::shared_ptr<wxBitmap> mCurrentBitmap;

    int mWidth;
    int mHeight;
    bool mDrawBoundingBox;
    wxBitmapPtr mBufferBitmap;

    boost::scoped_ptr<boost::thread> mVideoBufferThreadPtr;
    void videoBufferThread();

    boost::scoped_ptr<boost::thread> mVideoDisplayThreadPtr;
    void videoDisplayThread();

    /// safeguards access to the currently shown bitmap (mCurrentBitmap).
    boost::mutex mMutexDraw;

    //////////////////////////////////////////////////////////////////////////
    // GUI METHODS
    //////////////////////////////////////////////////////////////////////////

    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
};

} // namespace

#endif // GUI_VIDEO_DISPLAY_H