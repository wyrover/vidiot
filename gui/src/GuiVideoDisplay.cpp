#include "GuiVideoDisplay.h"

#include <algorithm>
#include <iomanip>
#include <boost/make_shared.hpp>
#include "wxInclude.h"
#include "UtilLog.h"
#include "IControl.h"
#include "GuiMain.h"
#include "GuiTimeLineZoom.h"
#include "Sequence.h"
#include <ostream>
#include "portaudio.h"
#include "FrameRate.h"

namespace gui {

DEFINE_EVENT(GUI_EVENT_PLAYBACK_POSITION, GuiEventPlaybackPosition, long);

const int GuiVideoDisplay::sStereo = 2;
const int GuiVideoDisplay::sFrameRate = 44100;
const int GuiVideoDisplay::sChannels = GuiVideoDisplay::sStereo;
const int GuiVideoDisplay::sBytesPerSample = 2;
const int GuiVideoDisplay::sVideoFrameRate = 25;

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
    bool cont = static_cast<GuiVideoDisplay*>(userData)->audioRequested(outputBuffer, framesPerBuffer, timeInfo->outputBufferDacTime);
    return cont ? 0 : paComplete;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiVideoDisplay::GuiVideoDisplay(wxWindow *parent, model::SequencePtr producer)
:   wxControl(parent, wxID_ANY)
,	mWidth(200)
,	mHeight(100)
,   mPlaying(false)
,	mProducer(producer)
,   mVideoFrames(FifoVideo(20))
,   mAudioChunks(FifoAudio(1000))
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
    GetClientSize(&mWidth,&mHeight);
    VAR_DEBUG(mWidth)(mHeight);

    Bind(wxEVT_PAINT,               &GuiVideoDisplay::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &GuiVideoDisplay::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &GuiVideoDisplay::OnSize,               this);

    PaError err = Pa_Initialize();
    ASSERT(err == paNoError)(Pa_GetErrorText(err));

	LOG_INFO;
}

GuiVideoDisplay::~GuiVideoDisplay()
{
    Unbind(wxEVT_PAINT,               &GuiVideoDisplay::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &GuiVideoDisplay::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &GuiVideoDisplay::OnSize,               this);

    moveTo(0); // stops playback

    PaError err = Pa_Terminate();
    ASSERT(err == paNoError)(Pa_GetErrorText(err));
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

void GuiVideoDisplay::play()
{
    if (mPlaying) return;

    // Ensure that the to-be-started threads do not immediately stop
    mAbortThreads = false;

    // Start buffering ASAP
    mAudioBufferThreadPtr.reset(new boost::thread(boost::bind(&GuiVideoDisplay::audioBufferThread,this)));
    mVideoBufferThreadPtr.reset(new boost::thread(boost::bind(&GuiVideoDisplay::videoBufferThread,this)));

    mStartTime = 0;     // This blocks displaying of video until signaled by the audio thread
    mCurrentTime = 0;   // Updates the displayed time
    mVideoDisplayThreadPtr.reset(new boost::thread(boost::bind(&GuiVideoDisplay::videoDisplayThread,this)));

    mCurrentAudioChunk.reset();

    PaError err = Pa_OpenDefaultStream( &mAudioOutputStream, 0, 2, paInt16, sFrameRate, paFramesPerBufferUnspecified, portaudio_callback, this );
    ASSERT(err == paNoError)(Pa_GetErrorText(err));

    err = Pa_StartStream( mAudioOutputStream );
    ASSERT(err == paNoError)(Pa_GetErrorText(err));

    mPlaying = true;

    LOG_DEBUG << "Playback started";
}

void GuiVideoDisplay::moveTo(int64_t position)
{
    VAR_DEBUG(this)(position);

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

        mAudioChunks.push(AudioChunkPtr()); // Unblock 'pop()', if needed
        mVideoFrames.push(VideoFramePtr()); // Unblock 'pop()', if needed
        mVideoDisplayThreadPtr->join();

        // Clear the buffers for a next run
        mVideoFrames.flush();
        mAudioChunks.flush();

        mPlaying = false;

        LOG_DEBUG << "Playback stopped";
    }

    // This must be done AFTER stopping all player threads, since - for instance -
    // otherwise the Track::moveTo() can interfere with Track::getNext...() when
    // changing the iterator.
    mProducer->moveTo(position); 
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

//////////////////////////////////////////////////////////////////////////
// AUDIO METHODS
//////////////////////////////////////////////////////////////////////////

void GuiVideoDisplay::audioBufferThread()
{
    while (!mAbortThreads) 
	{
        AudioChunkPtr chunk = mProducer->getNextAudio(sFrameRate,sStereo);
        mAudioChunks.push(chunk);
	}
}

bool GuiVideoDisplay::audioRequested(void *buffer, unsigned long frames, double playtime)
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

    static const unsigned int sSamplesPerStereoFrame = 2;
    unsigned long remainingSamples = frames * sSamplesPerStereoFrame;
    int16_t* out = static_cast<int16_t*>(buffer);

    while (remainingSamples > 0)
    {
        if (!mCurrentAudioChunk || (mCurrentAudioChunk->getUnreadSampleCount() == 0))
        {
            // Only get new chunk when the previous chunk was completely finished.

            if (mAbortThreads)
            {
                memset(out,0,remainingSamples * AudioChunk::sBytesPerSample); // * 2: bytes vs int16
                LOG_DEBUG << "Abort";
                return false;
            }

            if (mAudioChunks.getSize() == 0)
            {
                // When there is no new chunk, do not block, but return silence.
                memset(out,0,remainingSamples * AudioChunk::sBytesPerSample); // * 2: bytes vs int16
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

        unsigned long nSamples = std::min(remainingSamples, mCurrentAudioChunk->getUnreadSampleCount());

        memcpy(out,mCurrentAudioChunk->getUnreadSamples(),nSamples * AudioChunk::sBytesPerSample);
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

void GuiVideoDisplay::videoBufferThread()
{
	LOG_INFO;
    while (!mAbortThreads) 
	{
        VideoFramePtr videoFrame = mProducer->getNextVideo(mWidth,mHeight,false);
        mVideoFrames.push(videoFrame);
	}
}

void GuiVideoDisplay::videoDisplayThread()
{
    boost::unique_lock<boost::mutex> lock(mMutexPlaybackStarted);
    while (mStartTime == 0)
    {
        conditionPlaybackStarted.wait(lock);
    }
    LOG_INFO;

    while (!mAbortThreads) 
    {
        VideoFramePtr videoFrame = mVideoFrames.pop();
        if (!videoFrame)
        {
            // End of video reached
            return;
        }

        //////////////////////////////////////////////////////////////////////////
        // SCHEDULE NEXT REFRESH
        //////////////////////////////////////////////////////////////////////////

        mCurrentTime = convertPortAudioTime(Pa_GetStreamTime(mAudioOutputStream)) - mStartTime;
        int nextFrameTime = gui::timeline::GuiTimeLineZoom::ptsToTime(videoFrame->getPts() - mStartPts);
        int sleepTime = nextFrameTime - mCurrentTime;
        
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
}

//////////////////////////////////////////////////////////////////////////
// GUI METHODS
//////////////////////////////////////////////////////////////////////////

inline void GuiVideoDisplay::OnEraseBackground(wxEraseEvent& event)
{ 
    /* do nothing */ 
}

void GuiVideoDisplay::OnSize(wxSizeEvent& event) 
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

void GuiVideoDisplay::OnPaint(wxPaintEvent& event) 
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

void GuiVideoDisplay::showNewVideoFrame()
{
    Refresh(false);
    if (mCurrentVideoFrame)
    {
        // If there is no displayed video frame, do not change the timeline's cursor
        // position. An example of this is the case where the cursor is positioned 
        // beyond the end of the sequence.
        GetEventHandler()->QueueEvent(new GuiEventPlaybackPosition(mCurrentVideoFrame->getPts()));
    }
}

} // namespace
