// Copyright 2013-2015 Eric Raijmakers.
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

static int portaudio_callback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    if (statusFlags != 0)
    {
        if      (statusFlags & paInputUnderflow)  { LOG_WARNING << "paInputUnderflow"; }
        else if (statusFlags & paInputOverflow)   { LOG_WARNING << "paInputOverflow"; }
        else if (statusFlags & paOutputUnderflow) { LOG_WARNING << "paOutputUnderflow"; }
        else if (statusFlags & paOutputOverflow)  { LOG_WARNING << "paOutputOverflow"; }
        else if (statusFlags & paPrimingOutput)   { LOG_WARNING << "paPrimingOutput"; }
        else                                      { LOG_WARNING << "Unknown PaStreamCallbackFlags (" << static_cast<long>(statusFlags) << ")"; }
    }
    bool cont = static_cast<VideoDisplay*>(userData)->audioRequested(outputBuffer, framesPerBuffer, timeInfo->outputBufferDacTime);
    return cont ? paContinue : paAbort;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

VideoDisplay::VideoDisplay(wxWindow *parent, model::SequencePtr sequence)
    : wxControl(parent, wxID_ANY)
    , mWidth(200)
    , mHeight(100)
    , mPlaying(false)
    , mSequence(sequence)
    , mVideoFrames(200)
    , mAudioChunks(1000)
    , mAbortThreads(false)
    , mAudioBufferThreadPtr(0)
    , mVideoBufferThreadPtr(0)
    , mVideoTimer()
    , mCurrentAudioChunk()
    , mCurrentBitmap()
    , mStartTime(0)
    , mAudioLatency(0)
    , mStartPts(0)
    , mNumberOfAudioChannels(model::Properties::get().getAudioNumberOfChannels())
    , mAudioSampleRate(model::Properties::get().getAudioSampleRate())
    , mSkipFrames(0)
    , mSpeed(sDefaultSpeed)
{
    VAR_DEBUG(this);

    SetBackgroundStyle(wxBG_STYLE_PAINT); // Required for wxAutoBufferedPaintDC
    mBufferBitmap.reset(new wxBitmap(GetSize()));

    GetClientSize(&mWidth,&mHeight);
    VAR_DEBUG(mWidth)(mHeight);

    Bind(wxEVT_PAINT,               &VideoDisplay::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &VideoDisplay::onSize,               this);
    mVideoTimer.Bind(wxEVT_TIMER,   &VideoDisplay::onTimer,              this);

    LOG_INFO;
}

VideoDisplay::~VideoDisplay()
{
    VAR_DEBUG(this);

    Unbind(wxEVT_PAINT,               &VideoDisplay::onPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::onEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &VideoDisplay::onSize,               this);
    mVideoTimer.Unbind(wxEVT_TIMER,   &VideoDisplay::onTimer,              this);

    stop(); // stops playback
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::play()
{
    if (!mPlaying)
    {
        ASSERT(wxThread::IsMain());
        ASSERT(!mVideoBufferThreadPtr);
        ASSERT(!mAudioBufferThreadPtr);
        VAR_DEBUG(this);

        // Ensure that the to-be-started threads do not immediately stop
        mAbortThreads = false;

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
        try
        {
            mAudioBufferThreadPtr.reset(new boost::thread(boost::bind(&VideoDisplay::audioBufferThread,this)));
            mVideoBufferThreadPtr.reset(new boost::thread(boost::bind(&VideoDisplay::videoBufferThread,this)));
        }
        catch (boost::exception &e)
        {
            FATAL(boost::diagnostic_information(e));
        }

        mStartTime = 0;     // This blocks displaying of video until signaled by the audio thread
        mAudioLatency = 0;

        mCurrentAudioChunk.reset();

        PaError err = Pa_OpenDefaultStream( &mAudioOutputStream, 0, mNumberOfAudioChannels, paInt16, model::Properties::get().getAudioSampleRate(), paFramesPerBufferUnspecified, portaudio_callback, this );
        ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

        err = Pa_StartStream( mAudioOutputStream );
        ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

        //const PaStreamInfo* info = Pa_GetStreamInfo(mAudioOutputStream);

        mStartTime = Pa_GetStreamTime(mAudioOutputStream);
        mStartPts = (mCurrentVideoFrame ? mCurrentVideoFrame->getPts() : 0); // Used for determining inter frame sleep time

        showNextFrame();

        mPlaying = true;
        GetEventHandler()->QueueEvent(new PlaybackActiveEvent(true));

        LOG_DEBUG;
    }
}

void VideoDisplay::stop()
{
    VAR_DEBUG(this);
    ASSERT(wxThread::IsMain());

    mAbortThreads = true; // Stop getting new video/audio data

#ifdef __GNUC__
    //todo GCC get hangup if I do abortstream and closestream immediately after timer.stop....
    boost::this_thread::sleep(boost::posix_time::milliseconds(250));
#endif

    if (mPlaying)
    {
        LOG_DEBUG << "Playback stopping";

        mVideoTimer.Stop();

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

        // Clear the buffers for a next run
        mVideoFrames.flush();
        mAudioChunks.flush();

        mVideoBufferThreadPtr.reset();
        mAudioBufferThreadPtr.reset();

        mPlaying = false;
        GetEventHandler()->QueueEvent(new PlaybackActiveEvent(false));

        LOG_DEBUG << "Playback stopped";
    }
}

void VideoDisplay::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
    ASSERT(wxThread::IsMain());

    stop(); // Stop playback

    // This must be done AFTER stopping all player threads, since - for instance -
    // otherwise the Track::moveTo() can interfere with Track::getNext...() when
    // changing the iterator.
    mSequence->moveTo(position);
    GetEventHandler()->QueueEvent(new PlaybackPositionEvent(position));

    mCurrentVideoFrame = mSequence->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(mWidth,mHeight)));
    if (mCurrentVideoFrame)
    {
        mCurrentBitmap = mCurrentVideoFrame->getBitmap();
    }
    else
    {
        mCurrentBitmap.reset();
    }

    Refresh(false); // Update the shown bitmap
    Update(); // For immediate feedback when moving the cursor quickly over the timeline
}

void VideoDisplay::setSpeed(int speed)
{
    ASSERT(wxThread::IsMain());
    bool wasPlaying = mPlaying;
    mSpeed = speed;
#ifdef __GNUC__
        // todo GCC make SoundTouch work under linux (probably caused by wrong sample format, which I fixed to 2 bytes i.s.o. default float)
        mSpeed = sDefaultSpeed;
#endif
    moveTo(mCurrentVideoFrame ? mCurrentVideoFrame->getPts() : 0);
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
            if (mSpeed != sDefaultSpeed)
            {
                // In SoundTouch context a sample is the data for both speakers.
                // In AudioChunk it's the data for one speaker.
                // Soundtouch needs at least 4 * 2 bytes, see RateTransposerInteger::transposeStereo (uses src[3] - which is the fourth sample).
                //
                // To avoid access violations, too small chunks are skipped (given their size that should pose no problems for previewing only).
                static const samplecount sMinimumChunkSizeInSamples = 4;
                if (chunk->getUnreadSampleCount() < sMinimumChunkSizeInSamples) { continue; } // Skip this chunk

                //if (chunk->getUnreadSampleCount() < 3) continue;
                mSoundTouch.putSamples(reinterpret_cast<const soundtouch::SAMPLETYPE *>(chunk->getUnreadSamples()), chunk->getUnreadSampleCount() / mNumberOfAudioChannels) ;
                while (!mSoundTouch.isEmpty())
                {
                    int nFramesAvailable = mSoundTouch.numSamples();
                    sample* p = 0;
                    model::AudioChunkPtr audioChunk = boost::make_shared<model::AudioChunk>(mNumberOfAudioChannels, nFramesAvailable * mNumberOfAudioChannels, true, false);
                    int nFrames = mSoundTouch.receiveSamples(reinterpret_cast<soundtouch::SAMPLETYPE *>(audioChunk->getBuffer()), nFramesAvailable);
                    ASSERT_EQUALS(nFrames,nFramesAvailable);
                    audioChunk->setPts(chunk->getPts());
                    mAudioChunks.push(audioChunk);
                }
            }
            else
            {
                mAudioChunks.push(chunk); // No speed change. Just insert chunk.
            }
        }
        else
        {
            mAudioChunks.push(chunk); // Signal end
        }
    }
}

bool VideoDisplay::audioRequested(void *buffer, const unsigned long& frames, double playtime)
{
    samplecount remainingSamples = frames * mNumberOfAudioChannels;
    sample* out = static_cast<sample*>(buffer);

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

            double actualPlaytime = playtime - mStartTime;
            double expectedPlaytime =
                model::Convert::ptsToSeconds(mCurrentAudioChunk->getPts() - mStartPts) + 
                model::Convert::samplesToSeconds(remainingSamples);
            mAudioLatency = actualPlaytime - expectedPlaytime;
        }

        samplecount nSamples = mCurrentAudioChunk->extract(out,remainingSamples);
        ASSERT_MORE_THAN_EQUALS(remainingSamples,nSamples);
        remainingSamples -= nSamples;
        out += nSamples;
    }
    return !mAbortThreads;
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
        int nSkip = mSkipFrames.load();
        bool skip = nSkip > 0;
        model::VideoFramePtr videoFrame = mSequence->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(mWidth,mHeight)).setSkip(skip));
        if (!skip)
        {
            videoFrame->getBitmap(); // put in cache (avoid having to draw in GUI thread)
            mVideoFrames.push(videoFrame);
        }
        else
        {
            mSkipFrames.store(nSkip - 1);
        }
    }
}

void VideoDisplay::showNextFrame()
{
    if (mAbortThreads)
    {
        return;
    }

    model::VideoFramePtr videoFrame = mVideoFrames.pop();
    if (!videoFrame)
    {
        // End of video reached
        return;
    }

    double elapsed = Pa_GetStreamTime(mAudioOutputStream) - mStartTime - mAudioLatency; // Elapsed time since playback started
    double speedfactor = static_cast<double>(sDefaultSpeed) / static_cast<double>(mSpeed);
    double next = speedfactor * static_cast<double>(model::Convert::ptsToTime(videoFrame->getPts() - mStartPts)) / 1000.0; // time at which the frame must be shown; /1000.0: convert ms to s
    int sleep = static_cast<int>(floor((next - elapsed) * 1000.0));

    //LOG_ERROR << "Current time: " << std::fixed << elapsed;
    //LOG_ERROR << "Time to show next frame: " << std::fixed << next;
    //LOG_ERROR << "Sleep time:" << std::fixed << sleep;

    static const int sMinimumSleepTime = 0;
    if (sleep < sMinimumSleepTime)
    {
        // Too late, skip the picture

        if (mVideoFrames.getSize() > 0)
        {
            mVideoTimer.Notify(); // Immediately try next frame in buffer
            return;
        }

        VAR_WARNING(mVideoFrames.getSize())(mStartTime)(mAudioLatency)(elapsed)(next)(sleep)(mStartPts)(videoFrame->getPts());

        int skip = mSkipFrames.load();
        if (skip == 0)
        {
            mSkipFrames.store(5);
        }
        else
        {
            mSkipFrames.store(skip * 2);
        }

        sleep = model::Convert::ptsToTime(1);
    }
    else
    {
        mSkipFrames.store(0);

        mCurrentVideoFrame = videoFrame;
        mCurrentBitmap = videoFrame->getBitmap();

        // Note:
        // If there is no displayed video frame, do not change the timeline's cursor
        // position. An example of this is the case where the cursor is positioned
        // beyond the end of the sequence.
        //
        // Furthermore, when not playing, do not generate events. Otherwise, any cursor
        // move in the timeline (home/end/prevclip/nextclip) will cause such an event.
        // In turn, that might cause a change of the scrolling in Cursor::onPlaybackPosition
        // which is not desired for 'user initiated moves'.
        GetEventHandler()->QueueEvent(new PlaybackPositionEvent(videoFrame->getPts()));

        Refresh(false);
    }
    mVideoTimer.StartOnce(sleep);
}

//////////////////////////////////////////////////////////////////////////
// GUI METHODS
//////////////////////////////////////////////////////////////////////////

inline void VideoDisplay::onEraseBackground(wxEraseEvent& event)
{
    // do nothing
}

void VideoDisplay::onSize(wxSizeEvent& event)
{
    int w = mWidth;
    int h = mHeight;
    GetClientSize(&w,&h);

    if (mWidth != w || mHeight != h)
    {
        mWidth = w;
        mHeight = h;
        VAR_INFO(mWidth)(mHeight);

        if (mWidth > 0 && mHeight > 0) // With wxGTK sometimes w > 0 and h == 0 (during creation)
        {
            mBufferBitmap.reset(new wxBitmap(GetSize()));

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
}

void VideoDisplay::onPaint(wxPaintEvent& event)
{
    // Buffered dc is used, since first the entire area is blanked with drawrectangle,
    // and then overwritten. Without buffering that causes flickering.
    boost::scoped_ptr<wxDC> dc;
    if (!IsDoubleBuffered())
    {
        // A dedicated buffer bitmap is used. Without it I had conflicts between the buffered
        // bitmap used for VideoDisplay and Timeline: when pressing 'b' (trim begin) during
        // playback, one of the playback frames ended popping up over the timeline.
        dc.reset(new wxBufferedPaintDC(this, *mBufferBitmap, wxBUFFER_VIRTUAL_AREA ));  // See: http://trac.wxwidgets.org/ticket/15497
    }
    else
    {
        dc.reset(new wxPaintDC(this));
    }

    if (mCurrentBitmap)
    {
        // Don't use DrawBitmap since this gives wrong output when using wxGTK.
        wxMemoryDC dcBmp(*mCurrentBitmap);
        dc->Blit(0, 0, mCurrentBitmap->GetWidth(), mCurrentBitmap->GetHeight(), &dcBmp, 0, 0);
    }
    else
    {
        dc->SetPen(*wxBLACK);
        dc->SetBrush(*wxBLACK_BRUSH);
        dc->DrawRectangle(0, 0, mWidth, mHeight);
    }
}

void VideoDisplay::onTimer(wxTimerEvent& event)
{
    showNextFrame();
}

} // namespace
