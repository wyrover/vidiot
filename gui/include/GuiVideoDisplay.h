#ifndef GUI_VIDEO_DISPLAY_H
#define GUI_VIDEO_DISPLAY_H

#include <iostream>
#include <wx/panel.h>
#include <wx/control.h>
#include <boost/scoped_ptr.hpp>
#include <SoundTouch.h>
#include "VideoFrame.h"
#include "AudioChunk.h"
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "UtilEvent.h"

namespace gui {

DECLARE_EVENT(GUI_EVENT_PLAYBACK_POSITION, GuiEventPlaybackPosition, long);

class GuiVideoDisplay
:   public wxControl
{
public:

    static const int sStereo;
    static const int sFrameRate;
    static const int sChannels;
    static const int sBytesPerSample;
    static const int sVideoFrameRate;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiVideoDisplay(wxWindow *parent, model::SequencePtr producer);
	virtual ~GuiVideoDisplay();

    //////////////////////////////////////////////////////////////////////////
    // CONTROL METHODS
    //////////////////////////////////////////////////////////////////////////

    void play();
    void moveTo(int64_t position);
    void setSpeed(int speed);
    int getSpeed() const;

    //////////////////////////////////////////////////////////////////////////
    // AUDIO
    //////////////////////////////////////////////////////////////////////////

    /**
    * Called when portaudio needs more audio.
    * @param buffer target buffer to be filled
    * @param frames number of frames to be output.
    * @param playtime time at which this audio buffer will be played
    * @return true if more data is available, false if no more data is available
    * A frame is a combination of samples, one sample for each output channel.
    * Thus, a stereo frame contains a left and a right sample.
    * 
    * Method is public since it is called by the C callback.
    */
    bool audioRequested(void *buffer, unsigned long frames, double playtime);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    /** The Renderer responsible for producing video and audio data. */
	model::SequencePtr mProducer;

    /** Current playing speed */
    int mSpeed;

    /** If set, indicates that video and audio buffers must end. */
    bool mAbortThreads;

    /** true if there is currently a sequence being played. */
    bool mPlaying;

    /**
     * Holds the time at which the first audio buffer will be played.
     * Effectively, that's the start time of the playback. Time is in 
     * milliseconds.
     */
    int mStartTime;

    /**
    * Holds the pts at which the playback was started (thus, the 0-point
    * timewise)
    */
    int mStartPts;

    /**
    * Current time is updated on each new video frame.
    * This time is in milliseconds.
    */
    int mCurrentTime;

    /**
    * This mutex guards setting mStartTime in the audio playing thread
    * and reading it in the video display thread.
    */
    boost::mutex mMutexPlaybackStarted;

    /**
    * This condition signals the start of the playback. The playback is
    * started when the first audio buffer is requested and the start time is set.
    */
    boost::condition_variable conditionPlaybackStarted;

    //////////////////////////////////////////////////////////////////////////
    // AUDIO
    //////////////////////////////////////////////////////////////////////////

    model::FifoAudio mAudioChunks;
    model::AudioChunkPtr mCurrentAudioChunk;

    boost::scoped_ptr<boost::thread> mAudioBufferThreadPtr;
	void audioBufferThread();

    /** Required for portaudio */
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

    boost::scoped_ptr<boost::thread> mVideoBufferThreadPtr;
	void videoBufferThread();

    boost::scoped_ptr<boost::thread> mVideoDisplayThreadPtr;
	void videoDisplayThread();

    /** safeguards access to the currently shown bitmap (mCurrentBitmap). */
    boost::mutex mMutexDraw;

    /** Helper method. Triggers a position event and updates the bitmap. */
    void showNewVideoFrame();

    //////////////////////////////////////////////////////////////////////////
    // GUI METHODS
    //////////////////////////////////////////////////////////////////////////
    
    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
};

} // namespace

#endif // GUI_VIDEO_DISPLAY_H