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

#include <SoundTouch.h>
#include "VideoFrame.h"
#include "AudioChunk.h"

namespace model {
    class AudioCompositionParameters;
    class VideoCompositionParameters;
}

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
    void moveTo(pts position);

    ResumeInfo pause(pts position);
    void resume(const ResumeInfo& info);

    void setSpeed(int speed);
    int getSpeed() const;
    bool isPlaying() const;
    model::SequencePtr getSequence() const;

    void playRange(pts from, pts to);
    void stopRange();

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
    bool audioRequested(void *buffer, const unsigned long& frames, double playtime);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    /// The Renderer responsible for producing video and audio data.
    model::SequencePtr mSequence;

    /// Current playing speed
    int mSpeed;

    /// If set, indicates that video and audio buffers must end.
    std::atomic<bool> mAbortThreads;

    /// true if there is currently a sequence being played.
    bool mPlaying;

    /// Holds the time at which the first audio buffer will be played.
    /// Effectively, that's the start time of the playback. Time is in
    /// seconds since starting the audio playback.
    double mStartTime;

    /// Delta between the expected time at which audio is output and the
    /// actual time at which time are reported to be output by PortAudio.
    double mAudioLatency;

    /// Holds the pts at which the playback was started (thus, the 0-point timewise)
    pts mStartPts;

    std::atomic<int> mSkipFrames;

    boost::optional< std::pair<pts, pts> > mRange = boost::none;

    //////////////////////////////////////////////////////////////////////////
    // AUDIO
    //////////////////////////////////////////////////////////////////////////

    model::FifoAudio mAudioChunks;
    model::AudioChunkPtr mCurrentAudioChunk;

    std::unique_ptr<model::AudioCompositionParameters> mAudioParameters;
    boost::scoped_ptr<boost::thread> mAudioBufferThreadPtr;
    void sendToSoundTouch(model::AudioChunkPtr chunk);
    samplecount receiveFromSoundTouch(model::AudioChunkPtr chunk, samplecount nSamples, samplecount nSamplesRequired);
    void audioBufferThread();

    /// Required for portaudio
    void* mAudioOutputStream = nullptr;

    /// Required for SoundTouch
    soundtouch::SoundTouch mSoundTouch;
    std::atomic<samplecount> mSoundTouchLatency;
    double mSpeedFactor;

    //////////////////////////////////////////////////////////////////////////
    // VIDEO
    //////////////////////////////////////////////////////////////////////////

    model::FifoVideo mVideoFrames;
    model::VideoFramePtr mCurrentVideoFrame;
    boost::shared_ptr<wxBitmap> mCurrentBitmap;

    std::atomic<int> mWidth;
    std::atomic<int> mHeight;
    wxBitmapPtr mBufferBitmap = nullptr;

    std::unique_ptr<model::VideoCompositionParameters> mVideoParameters;
    boost::scoped_ptr<boost::thread> mVideoBufferThreadPtr;
    void videoBufferThread();

    void showNextFrame();

    wxTimer mVideoTimer;

    //////////////////////////////////////////////////////////////////////////
    // GUI METHODS
    //////////////////////////////////////////////////////////////////////////

    void onSize(wxSizeEvent& event);
    void onPaint(wxPaintEvent& event);
    void onEraseBackground(wxEraseEvent& event);
    void onTimer(wxTimerEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void stop();

    /// Update the parameters to be used for playback
    void updateParameters();

};

} // namespace
