#include "VideoDisplay.h"

#include <algorithm>
#include <iomanip>
#include <wx/dcclient.h>
#include <boost/make_shared.hpp>
#include <portaudio.h>
#include "UtilLog.h"
#include "Convert.h"
#include "Sequence.h"
#include "VideoDisplayEvent.h"

namespace gui {

const int VideoDisplay::sMinimumSpeed = 50;
const int VideoDisplay::sMaximumSpeed = 200;
const int VideoDisplay::sDefaultSpeed = 100;
const int VideoDisplay::sStereo = 2;
const int VideoDisplay::sFrameRate = 44100;
const int VideoDisplay::sChannels = VideoDisplay::sStereo;
const int VideoDisplay::sBytesPerSample = 2;
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

VideoDisplay::VideoDisplay(wxWindow *parent, model::SequencePtr producer)
:   wxControl(parent, wxID_ANY)
,	mWidth(200)
,	mHeight(100)
,   mPlaying(false)
,	mProducer(producer)
,   mVideoFrames(20)
,   mAudioChunks(1000)
,   mAbortThreads(false)
,   mAudioBufferThreadPtr(0)
,   mVideoBufferThreadPtr(0)
,   mVideoDisplayThreadPtr(0)
,   mCurrentAudioChunk()
,   mCurrentBitmap()
,   mStartTime(0)
,   mStartPts(0)
,   mCurrentTime(0)
{
    VAR_DEBUG(this);

    GetClientSize(&mWidth,&mHeight);
    VAR_DEBUG(mWidth)(mHeight);

    Bind(wxEVT_PAINT,               &VideoDisplay::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &VideoDisplay::OnSize,               this);

    PaError err = Pa_Initialize();
    ASSERT(err == paNoError)(Pa_GetErrorText(err));

	LOG_INFO;
}

VideoDisplay::~VideoDisplay()
{
    VAR_DEBUG(this);

    Unbind(wxEVT_PAINT,               &VideoDisplay::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &VideoDisplay::OnSize,               this);

    stop(); // stops playback

    PaError err = Pa_Terminate();
    ASSERT(err == paNoError)(Pa_GetErrorText(err));
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::play()
{
    VAR_DEBUG(this)(mPlaying);
    if (mPlaying) return;

    // Ensure that the to-be-started threads do not immediately stop
    mAbortThreads = false;

    // SoundTouch must be initialized before starting the audio buffer thread
    mSoundTouch.setSampleRate(sFrameRate);
    mSoundTouch.setChannels(sStereo);
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

    PaError err = Pa_OpenDefaultStream( &mAudioOutputStream, 0, 2, paInt16, sFrameRate, paFramesPerBufferUnspecified, portaudio_callback, this );
    ASSERT(err == paNoError)(Pa_GetErrorText(err));

    err = Pa_StartStream( mAudioOutputStream );
    ASSERT(err == paNoError)(Pa_GetErrorText(err));

    mPlaying = true;

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
        ASSERT(err == paNoError)(Pa_GetErrorText(err));

        err = Pa_CloseStream(mAudioOutputStream);
        ASSERT(err == paNoError)(Pa_GetErrorText(err));

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

        LOG_DEBUG << "Playback stopped";
    }
}

void VideoDisplay::moveTo(int64_t position)
{
    VAR_DEBUG(this)(position);

    stop(); // Stop playback

    // This must be done AFTER stopping all player threads, since - for instance -
    // otherwise the Track::moveTo() can interfere with Track::getNext...() when
    // changing the iterator.
    mProducer->moveTo(position);

    { // scoping for the lock: Update() below will cause a OnPaint which wants to take the lock.
        boost::mutex::scoped_lock lock(mMutexDraw);
        mCurrentVideoFrame = mProducer->getNextVideo(mWidth,mHeight,false);
        if (mCurrentVideoFrame)
        {
            mCurrentBitmap = boost::make_shared<wxBitmap>(wxImage(mCurrentVideoFrame->getWidth(), mCurrentVideoFrame->getHeight(), mCurrentVideoFrame->getData()[0], true));
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

//////////////////////////////////////////////////////////////////////////
// AUDIO METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::audioBufferThread()
{
    while (!mAbortThreads)
	{
        model::AudioChunkPtr chunk = mProducer->getNextAudio(sFrameRate,sStereo);

        if (chunk)
        {
            // In SoundTouch context a sample is the data for both speakers.
            // In AudioChunk it's the data for one speaker.
            mSoundTouch.putSamples(reinterpret_cast<const soundtouch::SAMPLETYPE *>(chunk->getUnreadSamples()), chunk->getUnreadSampleCount() / sStereo) ;
            while (!mSoundTouch.isEmpty())
            {
                int nFramesAvailable = mSoundTouch.numSamples();
                boost::int16_t* p = 0;
                model::AudioChunkPtr audioChunk = boost::make_shared<model::AudioChunk>(p, sStereo, nFramesAvailable * sStereo, 0);
                int nFrames = mSoundTouch.receiveSamples(reinterpret_cast<soundtouch::SAMPLETYPE *>(audioChunk->getBuffer()), nFramesAvailable);
                ASSERT(nFrames == nFramesAvailable)(nFrames)(nFramesAvailable);
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

    static const samples_t sSamplesPerStereoFrame = 2;
    samples_t remainingSamples = frames * sSamplesPerStereoFrame;
    int16_t* out = static_cast<int16_t*>(buffer);

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

        samples_t nSamples = min(remainingSamples, mCurrentAudioChunk->getUnreadSampleCount());

        memcpy(out,mCurrentAudioChunk->getUnreadSamples(),nSamples * model::AudioChunk::sBytesPerSample);
        mCurrentAudioChunk->read(nSamples);

        ASSERT(remainingSamples >= nSamples)(remainingSamples)(nSamples);
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
	LOG_INFO;
    while (!mAbortThreads)
	{
        model::VideoFramePtr videoFrame = mProducer->getNextVideo(mWidth,mHeight,false);
        mVideoFrames.push(videoFrame);
	}
}

void VideoDisplay::videoDisplayThread()
{
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

        mCurrentTime = convertPortAudioTime(Pa_GetStreamTime(mAudioOutputStream)) - mStartTime;
        int nextFrameTime = model::Convert::ptsToTime(videoFrame->getPts() - mStartPts);
        int nextFrameTimeAdaptedForPlaybackSpeed = (static_cast<float>(sDefaultSpeed) / static_cast<float>(mSpeed)) * static_cast<float>(nextFrameTime);
        int sleepTime = nextFrameTimeAdaptedForPlaybackSpeed - mCurrentTime;

        //////////////////////////////////////////////////////////////////////////
        // DISPLAY NEW FRAME
        //////////////////////////////////////////////////////////////////////////

        if (sleepTime < 20 || sleepTime > 1000)
        {
            // Skip the picture
            VAR_WARNING(sleepTime)(videoFrame->getPts());
            continue;
        }
        else
        {
            boost::mutex::scoped_lock lock(mMutexDraw);
            mCurrentVideoFrame = videoFrame;
            mCurrentBitmap = boost::make_shared<wxBitmap>(wxImage(mCurrentVideoFrame->getWidth(), mCurrentVideoFrame->getHeight(), mCurrentVideoFrame->getData()[0], true));
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

        if (!mPlaying && mCurrentVideoFrame)
        {
            moveTo(mCurrentVideoFrame->getPts());
        }
    }

}

void VideoDisplay::OnPaint(wxPaintEvent& event)
{
    boost::shared_ptr<wxBitmap> bitmap;
    {
        boost::mutex::scoped_lock lock(mMutexDraw);
        bitmap = mCurrentBitmap;
    }

    wxPaintDC dc(this); // Omit this, and suffer the performance consequences ;-)

    dc.SetPen(*wxBLACK);
    dc.SetBrush(*wxBLACK_BRUSH);
    if (bitmap)
    {
        if (bitmap->GetWidth() < mWidth)
        {
            dc.DrawRectangle( bitmap->GetWidth(), 0, mWidth - bitmap->GetWidth(), bitmap->GetHeight());
        }
        if (bitmap->GetHeight() < mHeight)
        {
            dc.DrawRectangle( 0, bitmap->GetHeight(), mWidth, mHeight - bitmap->GetHeight());
        }
        dc.DrawBitmap(*bitmap,wxPoint(0,0));
    }
    else
    {
        dc.DrawRectangle( 0, 0, mWidth, mHeight);
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
