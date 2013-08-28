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

#include "VideoDisplay.h"

#include "AudioCompositionParameters.h"
#include "Config.h"
#include "Convert.h"
#include "Layout.h"
#include "Properties.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "UtilThread.h"
#include "VideoCompositionParameters.h"
#include "VideoDisplayEvent.h"
#include <portaudio.h>

namespace gui {

const int VideoDisplay::sMinimumSpeed = 50;
const int VideoDisplay::sMaximumSpeed = 200;
const int VideoDisplay::sDefaultSpeed = 100;
const int VideoDisplay::sVideoFrameRate = 25;

int convertPortAudioTime(double patime)
{
    return static_cast<int>(floor(patime * 1000.0));
}

static int portaudio_callback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    if (statusFlags != 0)
    {
        VAR_WARNING(statusFlags);
    }
    bool cont = static_cast<VideoDisplay*>(userData)->audioRequested(outputBuffer, framesPerBuffer, timeInfo->outputBufferDacTime);
    return cont ? 0 : paComplete;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

VideoDisplay::VideoDisplay(wxWindow *parent, model::SequencePtr sequence)
:   wxControl(parent, wxID_ANY)
,	mWidth(200)
,	mHeight(100)
,   mDrawBoundingBox(false)
,   mPlaying(false)
,	mSequence(sequence)
,   mVideoFrames(20)
,   mAudioChunks(1000)
,   mAbortThreads(false)
,   mAudioBufferThreadPtr(0)
,   mVideoBufferThreadPtr(0)
,   mVideoDisplayThreadPtr(0)
,   mCurrentAudioChunk()
,   mCurrentBitmap()
,   mCurrentBitmapPosition(0,0)
,   mStartTime(0)
,   mStartPts(0)
,   mCurrentTime(0)
,   mNumberOfAudioChannels(model::Properties::get().getAudioNumberOfChannels())
,   mAudioSampleRate(model::Properties::get().getAudioFrameRate())
{
    VAR_DEBUG(this);

    SetBackgroundStyle(wxBG_STYLE_PAINT); // Required for wxAutoBufferedPaintDC

    GetClientSize(&mWidth,&mHeight);
    VAR_DEBUG(mWidth)(mHeight);

    Bind(wxEVT_PAINT,               &VideoDisplay::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &VideoDisplay::OnSize,               this);

    LOG_INFO;
}

VideoDisplay::~VideoDisplay()
{
    VAR_DEBUG(this);

    Unbind(wxEVT_PAINT,               &VideoDisplay::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &VideoDisplay::OnSize,               this);

    stop(); // stops playback
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::play()
{
    ASSERT(!mPlaying);
    VAR_DEBUG(this);

    // Ensure that the to-be-started threads do not immediately stop
    mAbortThreads = false;

    // Re-read every time the playback is restarted
    mDrawBoundingBox = Config::ReadBool(Config::sPathShowBoundingBox);

    // SoundTouch must be initialized before starting the audio buffer thread
    mSoundTouch.setSampleRate(mAudioSampleRate);
    mSoundTouch.setChannels(mNumberOfAudioChannels);
    mSoundTouch.setTempo(1.0);
    mSoundTouch.setTempoChange(mSpeed - sDefaultSpeed);
    mSoundTouch.setRate(1.0);
    mSoundTouch.setRateChange(0);
    mSoundTouch.setSetting(SETTING_USE_AA_FILTER, 0);//1
    mSoundTouch.setSetting(SETTING_SEQUENCE_MS, 40);    // Optimize for speech
    mSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);  // Optimize for speech
    mSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);      // Optimize for speech

    // Start buffering ASAP
    mAudioBufferThreadPtr.reset(new boost::thread(boost::bind(&VideoDisplay::audioBufferThread,this)));
    mVideoBufferThreadPtr.reset(new boost::thread(boost::bind(&VideoDisplay::videoBufferThread,this)));

    mStartTime = 0;     // This blocks displaying of video until signaled by the audio thread
    mCurrentTime = 0;   // Updates the displayed time
    mVideoDisplayThreadPtr.reset(new boost::thread(boost::bind(&VideoDisplay::videoDisplayThread,this)));

    mCurrentAudioChunk.reset();

    PaError err = Pa_OpenDefaultStream( &mAudioOutputStream, 0, mNumberOfAudioChannels, paInt16, model::Properties::get().getAudioFrameRate(), paFramesPerBufferUnspecified, portaudio_callback, this );
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

    err = Pa_StartStream( mAudioOutputStream );
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

    mPlaying = true;
    GetEventHandler()->QueueEvent(new PlaybackActiveEvent(true));

    LOG_DEBUG;
}

void VideoDisplay::stop()
{
    VAR_DEBUG(this);

    mAbortThreads = true; // Stop getting new video/audio data

    if (mPlaying)
    {
        LOG_DEBUG << "Playback stopping";

        // Stop audio
        PaError err = Pa_AbortStream(mAudioOutputStream);
        ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

        err = Pa_CloseStream(mAudioOutputStream);
        ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

        // End buffer threads
        mVideoFrames.flush(); // Unblock 'push()', if needed
        mAudioChunks.flush(); // Unblock 'push()', if needed
        mVideoBufferThreadPtr->join(); // One extra frame may have been inserted by 'push()'
        mAudioBufferThreadPtr->join(); // One extra chunk may have been inserted by 'push()'

        mSoundTouch.clear(); // Must be done after joining the audio buffer thread.

        mAudioChunks.push(model::AudioChunkPtr()); // Unblock 'pop()', if needed
        mVideoFrames.push(model::VideoFramePtr()); // Unblock 'pop()', if needed

        // Playback can be stopped (moveTo) before the first audioRequested is done by portaudio.
        // That shouldn't block the videoDisplayThread (which blocks until audioRequested signals
        // it to proceed).
        mStartTime = -1; // When the videoDisplayThread is unblocked it shouldn't start waiting again (while loop)
        conditionPlaybackStarted.notify_one(); // Unblock the waiting thread

        mVideoDisplayThreadPtr->join();

        // Clear the buffers for a next run
        mVideoFrames.flush();
        mAudioChunks.flush();

        mPlaying = false;
        GetEventHandler()->QueueEvent(new PlaybackActiveEvent(false));

        LOG_DEBUG << "Playback stopped";
    }
}

void VideoDisplay::moveTo(pts position)
{
    VAR_DEBUG(this)(position);

    stop(); // Stop playback

    // This must be done AFTER stopping all player threads, since - for instance -
    // otherwise the Track::moveTo() can interfere with Track::getNext...() when
    // changing the iterator.
    mSequence->moveTo(position);

    // Re-read every time the playback is restarted. The value used when 'playing' must be the same as the value used here.
    mDrawBoundingBox = Config::ReadBool(Config::sPathShowBoundingBox);

    { // scoping for the lock: Update() below will cause a OnPaint which wants to take the lock.
        boost::mutex::scoped_lock lock(mMutexDraw);
        mCurrentVideoFrame = mSequence->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(mWidth,mHeight)).setDrawBoundingBox(mDrawBoundingBox));
        if (mCurrentVideoFrame)
        {
            mCurrentBitmap = mCurrentVideoFrame->getBitmap();
            mCurrentBitmapPosition = mCurrentVideoFrame->getPosition();
        }
        else
        {
            mCurrentBitmap.reset();
        }

        showNewVideoFrame();
    }
    Update(); // For immediate feedback when moving the cursor quickly over the timeline
}

void VideoDisplay::setSpeed(int speed)
{
    bool wasPlaying = mPlaying;
    mSpeed = speed;
    moveTo(mCurrentVideoFrame?mCurrentVideoFrame->getPts():0);
    if (wasPlaying)
    {
        play();
    }
}

int VideoDisplay::getSpeed() const
{
    return mSpeed;
}

bool VideoDisplay::isPlaying() const
{
    return mPlaying;
}

model::SequencePtr VideoDisplay::getSequence() const
{
    return mSequence;
}

//////////////////////////////////////////////////////////////////////////
// AUDIO METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::audioBufferThread()
{
    util::thread::setCurrentThreadName("AudioBufferThread");
    while (!mAbortThreads)
    {
        model::AudioChunkPtr chunk = mSequence->getNextAudio(model::AudioCompositionParameters().setSampleRate(mAudioSampleRate).setNrChannels(mNumberOfAudioChannels));

        if (chunk)
        {
            // In SoundTouch context a sample is the data for both speakers.
            // In AudioChunk it's the data for one speaker.
            mSoundTouch.putSamples(reinterpret_cast<const soundtouch::SAMPLETYPE *>(chunk->getUnreadSamples()), chunk->getUnreadSampleCount() / mNumberOfAudioChannels) ;
            while (!mSoundTouch.isEmpty())
            {
                int nFramesAvailable = mSoundTouch.numSamples();
                sample* p = 0;
                model::AudioChunkPtr audioChunk = boost::make_shared<model::AudioChunk>(mNumberOfAudioChannels, nFramesAvailable * mNumberOfAudioChannels, true, false);
                int nFrames = mSoundTouch.receiveSamples(reinterpret_cast<soundtouch::SAMPLETYPE *>(audioChunk->getBuffer()), nFramesAvailable);
                ASSERT_EQUALS(nFrames,nFramesAvailable);
                mAudioChunks.push(audioChunk);
            }
        }
        else
        {
            mAudioChunks.push(chunk); // Signal end
        }
    }
}

bool VideoDisplay::audioRequested(void *buffer, unsigned long frames, double playtime)
{
    if (mStartTime == 0)
    {
        {
            boost::lock_guard<boost::mutex> lock(mMutexPlaybackStarted);
            mStartTime = convertPortAudioTime(playtime);
            mStartPts = (mCurrentVideoFrame?mCurrentVideoFrame->getPts():0); // Used for determining inter frame sleep time
        }
        conditionPlaybackStarted.notify_one();
    }

    samplecount remainingSamples = frames * mNumberOfAudioChannels;
    uint16_t* out = static_cast<uint16_t*>(buffer);

    while (remainingSamples > 0)
    {
        if (!mCurrentAudioChunk || (mCurrentAudioChunk->getUnreadSampleCount() == 0))
        {
            // Only get new chunk when the previous chunk was completely finished.

            if (mAbortThreads)
            {
                memset(out,0,remainingSamples * model::AudioChunk::sBytesPerSample); // * 2: bytes vs int16
                LOG_DEBUG << "Abort";
                return false;
            }

            if (mAudioChunks.getSize() == 0)
            {
                // When there is no new chunk, do not block, but return silence.
                memset(out,0,remainingSamples * model::AudioChunk::sBytesPerSample); // * 2: bytes vs int16
                LOG_WARNING << "Underflow";
                return true;
            }

            mCurrentAudioChunk = mAudioChunks.pop();
            if (!mCurrentAudioChunk)
            {
                LOG_INFO << "End";
                return false;
            }
        }

        samplecount nSamples = mCurrentAudioChunk->extract(out,remainingSamples);
        ASSERT_MORE_THAN_EQUALS(remainingSamples,nSamples);
        remainingSamples -= nSamples;
        out += nSamples;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// VIDEO METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::videoBufferThread()
{
    util::thread::setCurrentThreadName("VideoBufferThread");
    LOG_INFO;
    while (!mAbortThreads)
    {
        model::VideoFramePtr videoFrame = mSequence->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(mWidth,mHeight)).setDrawBoundingBox(mDrawBoundingBox));
        mVideoFrames.push(videoFrame);
    }
}

void VideoDisplay::videoDisplayThread()
{
    util::thread::setCurrentThreadName("VideoDisplayThread");
    boost::unique_lock<boost::mutex> lock(mMutexPlaybackStarted);
    while (mStartTime == 0)
    {
        conditionPlaybackStarted.wait(lock);
    }
    LOG_INFO;

    while (!mAbortThreads)
    {
        model::VideoFramePtr videoFrame = mVideoFrames.pop();
        if (!videoFrame)
        {
            // End of video reached
            return;
        }

        //////////////////////////////////////////////////////////////////////////
        // SCHEDULE NEXT REFRESH
        //////////////////////////////////////////////////////////////////////////

        int paTime = convertPortAudioTime(Pa_GetStreamTime(mAudioOutputStream));
        mCurrentTime = paTime - mStartTime;
        int nextFrameTime = model::Convert::ptsToTime(videoFrame->getPts() - mStartPts);
        int nextFrameTimeAdaptedForPlaybackSpeed = (static_cast<float>(sDefaultSpeed) / static_cast<float>(mSpeed)) * static_cast<float>(nextFrameTime);
        int sleepTime = nextFrameTimeAdaptedForPlaybackSpeed - mCurrentTime;

        //////////////////////////////////////////////////////////////////////////
        // DISPLAY NEW FRAME
        //////////////////////////////////////////////////////////////////////////

        if (sleepTime < 0)
        {
            // Skip the picture
            VAR_WARNING(mVideoFrames.getSize())(paTime)(mStartTime)(mCurrentTime)(sleepTime)(nextFrameTime)(nextFrameTimeAdaptedForPlaybackSpeed)(mStartPts)(videoFrame->getPts());

            // Originally the if statement read: (sleepTime < 0|| sleepTime > 1000), and the following sleep was done:
            // boost::this_thread::sleep(boost::posix_time::milliseconds(sleepTime));
            // However, sleeping with a negative value caused stuttering playback. Hence this statement was removed.
            // Don't know why the if > 1000 was ever added though (that was probably the reason for adding the sleep).
            continue;
        }
        else
        {
            boost::mutex::scoped_lock lock(mMutexDraw);
            ASSERT_DIFFERS(mCurrentVideoFrame->getPts(), videoFrame->getPts()); // Otherwise, all the computations on 'playback time' can behave irratic.
            mCurrentVideoFrame = videoFrame;
            mCurrentBitmap = videoFrame->getBitmap();
            mCurrentBitmapPosition = videoFrame->getPosition();
            showNewVideoFrame();
            boost::this_thread::sleep(boost::posix_time::milliseconds(sleepTime));
        }

    }

    LOG_INFO << "Stopped";
}

//////////////////////////////////////////////////////////////////////////
// GUI METHODS
//////////////////////////////////////////////////////////////////////////

inline void VideoDisplay::OnEraseBackground(wxEraseEvent& event)
{
    // do nothing
}

void VideoDisplay::OnSize(wxSizeEvent& event)
{
    int w = mWidth;
    int h = mHeight;
    GetClientSize(&w,&h);

    if (mWidth != w || mHeight != h)
    {
        mWidth = w;
        mHeight = h;
        VAR_INFO(mWidth)(mHeight);

        if (mCurrentVideoFrame)
        {
            // Note: this is also done when playback of video has started.
            // This is done to avoid problems when resizing while playing
            // (problem: videocomposition parameters change between frames,
            // but the caching mechanisms inside the video classes cause a
            // previous frame to be used again - wrong size).
            moveTo(mCurrentVideoFrame->getPts());
        }
    }

}

void VideoDisplay::OnPaint(wxPaintEvent& event)
{
    wxBitmapPtr bitmap;
    wxPoint position;
    {
        boost::mutex::scoped_lock lock(mMutexDraw);
        bitmap = mCurrentBitmap;
        position = mCurrentBitmapPosition;
    }

    // Buffered dc is used, since first the entire area is blanked with drawrectangle, and then overwritten. Without buffering that causes flickering.
    wxAutoBufferedPaintDC dc(this); // Omit this, and suffer the performance consequences ;-)

    dc.SetPen(*wxBLACK);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle( 0, 0, mWidth, mHeight); // black bg
    if (bitmap)
    {
        dc.DrawBitmap(*bitmap,position);
    }
}

void VideoDisplay::showNewVideoFrame()
{
    Refresh(false);
    if (mCurrentVideoFrame)
    {
        // If there is no displayed video frame, do not change the timeline's cursor
        // position. An example of this is the case where the cursor is positioned
        // beyond the end of the sequence.
        GetEventHandler()->QueueEvent(new PlaybackPositionEvent(mCurrentVideoFrame->getPts()));
    }
}

} // namespace