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

#include "VideoDisplay.h"

#include "AudioCompositionParameters.h"
#include "Config.h"
#include "Convert.h"
#include "Dialog.h"
#include "Properties.h"
#include "Sequence.h"
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
    , mSkipFrames(0)
    , mSpeed(sDefaultSpeed)
    , mSoundTouchLatency(-1)
    , mSpeedFactor(1.0)
{
    VAR_DEBUG(this);

    SetBackgroundStyle(wxBG_STYLE_PAINT); // Required for wxAutoBufferedPaintDC

    int w{ mWidth };
    int h{ mHeight };
    GetClientSize(&w,&h);
    mWidth = w;
    mHeight = h;
    VAR_DEBUG(mWidth)(mHeight);

    if (mWidth > 0 && mHeight > 0) // With wxGTK sometimes w > 0 and h == 0 (during creation)
    {
        mBufferBitmap.reset(new wxBitmap(GetSize()));
    }

    Bind(wxEVT_PAINT,               &VideoDisplay::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &VideoDisplay::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &VideoDisplay::onSize,               this);
    mVideoTimer.Bind(wxEVT_TIMER,   &VideoDisplay::onTimer,              this);

    updateParameters();

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

        updateParameters();

        // SoundTouch must be initialized before starting the audio buffer thread
        mSpeedFactor = static_cast<double>(sDefaultSpeed) / static_cast<double>(mSpeed);
        mSoundTouch.setSampleRate(mAudioParameters->getSampleRate());
        mSoundTouch.setChannels(mAudioParameters->getNrChannels());
        mSoundTouch.setTempo(1.0);
        mSoundTouch.setTempoChange(mSpeed - sDefaultSpeed);
        mSoundTouch.setRate(1.0);
        mSoundTouch.setRateChange(0);
        mSoundTouch.setSetting(SETTING_USE_AA_FILTER, 0);//1
        mSoundTouch.setSetting(SETTING_SEQUENCE_MS, 40);    // Optimize for speech
        mSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);  // Optimize for speech
        mSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);      // Optimize for speech
        mSoundTouchLatency = -1;

        // Used for determining inter frame sleep time. Is used for the buffered
        // audio packets and is therefore initialized before starting the thread.
        mStartPts = (mCurrentVideoFrame ? mCurrentVideoFrame->getPts() : 0);

        // Start buffering ASAP
        try
        {
            mAudioBufferThreadPtr.reset(new boost::thread(std::bind(&VideoDisplay::audioBufferThread,this)));
            mVideoBufferThreadPtr.reset(new boost::thread(std::bind(&VideoDisplay::videoBufferThread,this)));
        }
        catch (boost::exception &e)
        {
            FATAL(boost::diagnostic_information(e));
        }

        mAudioLatency = 0;
        mCurrentAudioChunk.reset();

#ifdef __GNUC__
        // On Linux (Ubuntu), the default buffer size is around 383.
        // Current implementation locks too much to keep up.
        // todo use less locking and use the default number of frames per buffer for linux also.
        unsigned long bufferSize = model::Convert::ptsToSamplesPerChannel(mAudioParameters->getSampleRate(), 2);
#else
        unsigned long bufferSize = paFramesPerBufferUnspecified;
#endif // __GNUC__

        auto verify = [this](PaError err, wxString message)
        {
            if (err == paNoError) { return true; }
            wxString msg; msg << _("Could not initialize playback.") << "\n" << message << Pa_GetErrorText(err);
            VAR_ERROR(msg);
            gui::Dialog::get().getConfirmation("Error", msg);
            return false;
        };

        PaError err = Pa_OpenDefaultStream( &mAudioOutputStream, 0, mAudioParameters->getNrChannels(), paInt16, mAudioParameters->getSampleRate(), bufferSize, portaudio_callback, this );
        if (!verify(err, _("Opening audio stream failed:"))) 
        { 
            mAudioOutputStream = nullptr;
            mPlaying = true; // Ensure that 'stop' code is executed.
            stop();
            return;
        }

        mStartTime = Pa_GetStreamTime(mAudioOutputStream);

        err = Pa_StartStream( mAudioOutputStream );
        if (!verify(err, _("Starting audio stream failed:"))) 
        { 
            PaError err{ Pa_CloseStream(mAudioOutputStream) };
            VAR_ERROR(err)(Pa_GetErrorText(err));
            mAudioOutputStream = nullptr;
            mPlaying = true; // Ensure that 'stop' code is executed.
            stop();
            return;
        }

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

    if (mPlaying)
    {
        LOG_DEBUG << "Playback stopping";

        mVideoTimer.Stop();

        // Stop audio
        if (mAudioOutputStream != nullptr)
        {
            if (0 == Pa_IsStreamStopped(mAudioOutputStream))
            {
                // Do not use Pa_AbortStream here, that may lead to
                // hangups/crashes. Just start playback and then hold
                // the spacebar to continuously toggle playback start/stop.
                // Eventually, a crash/hangup occurs.
                //
                // That is probably caused by the stream not being
                // fully stopped after aborting. Then, the subsequent
                // Pa_CloseStream seems to trigger the crash/hangup.
                PaError err{ Pa_StopStream(mAudioOutputStream) };
                ASSERT_EQUALS(err, paNoError)(Pa_GetErrorText(err));
            }
            if (Pa_IsStreamStopped(mAudioOutputStream) != 1)
            {
                PaError err{ Pa_IsStreamStopped(mAudioOutputStream) };
                VAR_ERROR(err)(Pa_GetErrorText(err));
            }

            PaError err{ Pa_CloseStream(mAudioOutputStream) };
            ASSERT_EQUALS(err, paNoError)(Pa_GetErrorText(err));
        }
        mAudioOutputStream = nullptr;

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

    if (mWidth > 0 && mHeight > 0)
    {
        // Avoid getting bitmaps for empty windows during startup.
        mCurrentVideoFrame = mSequence->getNextVideo(mVideoParameters->setBoundingBox(wxSize(mWidth,mHeight)));
    }
    else
    {
        mCurrentVideoFrame.reset();
    }
    if (mCurrentVideoFrame)
    {
        mCurrentBitmap = mCurrentVideoFrame->getBitmap();
    }
    else
    {
        mCurrentBitmap.reset();
    }

    Refresh(true); // Update the shown bitmap
    Update(); // For immediate feedback when moving the cursor quickly over the timeline
}

void VideoDisplay::setSpeed(int speed)
{
    ASSERT(wxThread::IsMain());
    bool wasPlaying = mPlaying;
    mSpeed = speed;
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

void VideoDisplay::sendToSoundTouch(model::AudioChunkPtr chunk)
{
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    mSoundTouch.putSamples(reinterpret_cast<const short *>(chunk->getUnreadSamples()), chunk->getUnreadSampleCount() / mAudioParameters->getNrChannels());
#else
    float *convertTo = new float[chunk->getUnreadSampleCount()];
    sample* s = chunk->getUnreadSamples();
    float* t = convertTo;
    static const float M = 32767.0; // 2 ^ 15
    for (int i = 0; i < chunk->getUnreadSampleCount(); ++i)
    {
        *t++ = static_cast<float>(*s++) / M;
    }
    mSoundTouch.putSamples(convertTo, chunk->getUnreadSampleCount() / mAudioParameters->getNrChannels());
    delete[] convertTo;
#endif
}

samplecount VideoDisplay::receiveFromSoundTouch(model::AudioChunkPtr chunk, samplecount offset, samplecount nSamplesRequired)
{
    int nFramesAvailable = mSoundTouch.numSamples();
    int nFramesRequired = nSamplesRequired / mAudioParameters->getNrChannels();
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    samplecount nFrames = mSoundTouch.receiveSamples(reinterpret_cast<short*>(chunk->getBuffer()) + offset, nFramesRequired);
    ASSERT_LESS_THAN_EQUALS(nFrames, nFramesAvailable);
 #else // SOUNDTOUCH_FLOAT_SAMPLES
    static const float M = 32767.0; // 2 ^ 15
    float* convertFrom = new float[nSamplesRequired];
    samplecount nFrames = mSoundTouch.receiveSamples(convertFrom, nFramesRequired);
    ASSERT_LESS_THAN_EQUALS(nFrames, nFramesAvailable);
    ASSERT_LESS_THAN_EQUALS(nFrames, nFramesRequired);

    int nSamples = nFrames * mAudioParameters->getNrChannels();
    sample* t = chunk->getBuffer() + offset;
    float* s = convertFrom;
    for (int i = 0; i < nSamples; ++i)
    {
        float f = *s++ * M;
        if ( f < -M )  { f = -M; }
        if ( f > M-1 ) { f = M - 1; }
        *t++ = static_cast<sample>(f);
    }
    delete[] convertFrom;
#endif
    return nFrames * mAudioParameters->getNrChannels();
}

void VideoDisplay::audioBufferThread()
{
    util::thread::setCurrentThreadName("AudioBufferThread");
    model::AudioChunkPtr inputChunk;
    if (mSpeed == sDefaultSpeed)
    {
        while (!mAbortThreads)
        {
            model::AudioCompositionParameters parameters(*mAudioParameters);
            mAudioChunks.push(mSequence->getNextAudio(parameters)); // No speed change. Just insert chunk.
        }
    }
    else
    {
        bool atEnd = false;
        pts outputPts = mStartPts;
        while (!mAbortThreads)
        {
            samplecount chunksize = model::AudioCompositionParameters(*mAudioParameters).setPts(outputPts).determineChunkSize().getChunkSize();
            model::AudioChunkPtr outputChunk = boost::make_shared<model::AudioChunk>(mAudioParameters->getNrChannels(), chunksize, true, false);
            samplecount writtenSamples = 0;
            while (writtenSamples < chunksize)
            {
                if (!mSoundTouch.isEmpty())
                {
                    writtenSamples += receiveFromSoundTouch(outputChunk, writtenSamples, chunksize - writtenSamples);
                }
                else if (atEnd)
                {
                    mAudioChunks.push(model::AudioChunkPtr()); // Signal end
                    return;
                }
                else
                {
                    model::AudioCompositionParameters parameters(*mAudioParameters);
                    model::AudioChunkPtr chunk = mSequence->getNextAudio(parameters);
                    atEnd = chunk == nullptr;
                    if (!atEnd)
                    {
                        sendToSoundTouch(chunk);
                    }
                }
            }
            outputChunk->setPts(outputPts++);
            mAudioChunks.push(outputChunk);
        }
    }
}

bool VideoDisplay::audioRequested(void *buffer, const unsigned long& frames, double playtime)
{
    samplecount remainingSamples = frames * mAudioParameters->getNrChannels();
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
            int chunks = mAudioChunks.getSize(); // lock 1
            if (chunks == 0)
            {
                // When there is no new chunk, do not block, but return silence.
                memset(out,0,remainingSamples * model::AudioChunk::sBytesPerSample); // * 2: bytes vs int16
                LOG_WARNING << "Underflow";
                return true;
            }

            mCurrentAudioChunk = mAudioChunks.pop(); // lock 2
            if (!mCurrentAudioChunk)
            {
                LOG_INFO << "End";
                return false;
            }

            double actualPlaytime = playtime - mStartTime;
            double expectedPlaytime =
                (model::Convert::ptsToSeconds(mCurrentAudioChunk->getPts() - mStartPts) +
                 model::Convert::samplesToSeconds(remainingSamples));
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
        model::VideoFramePtr videoFrame = mSequence->getNextVideo(mVideoParameters->setBoundingBox(wxSize(mWidth,mHeight)).setSkip(skip));
        if (!skip)
        {
            // NOT: videoFrame->getBitmap(); // put in cache (avoid having to draw in GUI thread) -- Don't do this anymore since this is a gdi object in a separate thread.
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
    double next = static_cast<double>(model::Convert::ptsToSeconds(videoFrame->getPts() - mStartPts)) * mSpeedFactor; // time at which the frame must be shown; /1000.0: convert ms to s
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
    int w{ mWidth };
    int h{ mHeight };
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
    if (!IsDoubleBuffered() &&
        mBufferBitmap != nullptr)
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
        dc->SetPen(wxPen{wxColour{0,0,0}});
        dc->SetBrush(wxColour{0,0,0});
        dc->DrawRectangle(0, 0, mWidth, mHeight);
    }
}

void VideoDisplay::onTimer(wxTimerEvent& event)
{
    showNextFrame();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void VideoDisplay::updateParameters()
{
    ASSERT(wxThread::IsMain());
    mAudioParameters = std::make_unique<model::AudioCompositionParameters>();
    mVideoParameters = std::make_unique<model::VideoCompositionParameters>();
    mVideoParameters->setDrawBoundingBox(Config::ReadBool(Config::sPathVideoShowBoundingBox));
}

} // namespace
