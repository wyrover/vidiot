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

// Pts/Dts synchronization code:
// See http://dranger.com/ffmpeg/tutorial05.html

#include "VideoFile.h"

#include "Convert.h"
#include "Node.h"
#include "Properties.h"
#include "UtilInitAvcodec.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// AVCODEC CALLBACKS
//////////////////////////////////////////////////////////////////////////

int avcodec_get_buffer(struct AVCodecContext *c, AVFrame *pic, int flags)
{
    ASSERT_NONZERO(c);
    ASSERT_NONZERO(pic);
    int ret = avcodec_default_get_buffer2(c, pic, flags);
    int64_t *pts = static_cast<int64_t*>(av_malloc(sizeof(int64_t)));
    *pts = static_cast<VideoFile*>(c->opaque)->getVideoPacketPts();
    pic->opaque = pts;
    return ret;
}

static int const sMaxBufferSize = 10;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFile::VideoFile()
    :	File()
    ,   mDecodingVideo(false)
    ,   mDeliveredFrame()
    ,   mSwsContext(0)
    ,   mVideoPacketPts(AV_NOPTS_VALUE)
{
    VAR_DEBUG(*this);
}

VideoFile::VideoFile(const wxFileName& path)
    :	File(path,sMaxBufferSize)
    ,   mDecodingVideo(false)
    ,   mDeliveredFrame()
    ,   mSwsContext(0)
    ,   mVideoPacketPts(AV_NOPTS_VALUE)
{
    VAR_DEBUG(*this);
}

VideoFile::VideoFile(const VideoFile& other)
    :   File(other)
    ,   mDecodingVideo(false)
    ,   mDeliveredFrame()
    ,   mSwsContext(0)
    ,   mVideoPacketPts(AV_NOPTS_VALUE)
{
    VAR_DEBUG(*this);
}

VideoFile* VideoFile::clone() const
{
    return new VideoFile(static_cast<const VideoFile&>(*this));
}

VideoFile::~VideoFile()
{
    VAR_DEBUG(this);
    clean();
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void VideoFile::moveTo(pts position)
{
    stopDecodingVideo();

    mDeliveredFrame.reset();

    File::moveTo(position); // NOTE: This uses the pts in 'project' timebase units
}

void VideoFile::clean()
{
    VAR_DEBUG(this);

    stopDecodingVideo();

    mDeliveredFrame.reset();

    File::clean();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoFile::getNextVideo(const VideoCompositionParameters& parameters)
{
    startDecodingVideo(parameters);
    ASSERT(canBeOpened());
    AVPacket nullPacket;
    memset(&nullPacket,0,sizeof(AVPacket));

    AVCodecContext* codec = getCodec();
    ASSERT_NONZERO(avcodec_is_open(codec));

    ASSERT_ZERO(codec->refcounted_frames); // for new version of avcodec, see avcodec_decode_video2 docs

    wxSize codecSize(codec->width,getCodec()->height);
    wxSize size(Convert::sizeInBoundingBox(wxSize(codec->width,codec->height),parameters.getBoundingBox()));
    static const int sMinimumFrameSize = 10;        // I had issues when generating smaller bitmaps. To avoid these, always
    size.x = std::max(size.x,sMinimumFrameSize);    // use a minimum framesize. The region of interest in videoclips will ensure
    size.y = std::max(size.y,sMinimumFrameSize);    // that any excess data is cut off.

    // Pts/time based variables
    AVStream* stream = getStream();
    AVRational streamRate = av_stream_get_r_frame_rate(stream);
    FrameRate inputFrameRate = FrameRate(streamRate.num, streamRate.den); // 24000/1001
    FrameRate inputTimeBase = FrameRate(stream->time_base.num, stream->time_base.den); // 1/240000
    rational64 ticksPerFrame = rational64(1) / (inputFrameRate * inputTimeBase);
    ASSERT_MORE_THAN_ZERO(ticksPerFrame);

    auto positionToFrameTime = [this, inputTimeBase](pts inputPosition) -> rational64
    {
        return rational64(inputPosition) * inputTimeBase * 1000;
    };

    // 'Resample' the frame timebase
    // Determine which pts value is required. This is required to first determine
    // if the previously returned frame should be returned again
    auto frameTimeOk = [this, parameters, stream, inputTimeBase, ticksPerFrame, positionToFrameTime](pts inputPosition) -> bool
    {
        bool result = true;
        if (parameters.hasPts() &&
            inputPosition != AV_NOPTS_VALUE)
        {
            pts requiredPts{ parameters.getPts() };
            milliseconds requiredTime = Convert::ptsToTime(requiredPts); // Compute back to original speed
            if (stream->start_time != AV_NOPTS_VALUE)
            {
                // Some streams don't start counting at 0
                // NOTE: alternative might be stream->first_dts
                inputPosition -= stream->start_time;
            }
            rational64 nextTime = rational64(inputPosition + ticksPerFrame) * inputTimeBase * 1000;
            milliseconds diffCurrent = abs(requiredTime - boost::rational_cast<milliseconds>(positionToFrameTime(inputPosition)));
            milliseconds diffNext = abs(boost::rational_cast<milliseconds>(nextTime) - requiredTime);
            result =  diffCurrent < diffNext;
        }
        // else: Just deliver the first found frame.
        //       Typical case: directly after 'moveTo' during thumbnail generation,
        //       timeline cursor move, trim, etc. (basically, anything except playback)
        //       Other case: could not determine input frame position (AV_NOPTS_VALUE)
        return result;
    };

    // NOTE: Sometimes the gotten input frame covers two output frames.
    //       Typically seen with H264 - MPEG4AVC (part10) (avc1) files with a frame rate of 28 frames/s.
    // NOTE: Parameters can be changed during playback by - for instance - changing 'showBoundingBox'

    pts decodedFramePts = AV_NOPTS_VALUE;

    VideoFramePtr result = mDeliveredFrame;
    if (!result || !frameTimeOk(result->getPts()))
    {
        // Decode new frame
        int frameFinished = 0;
        AVFrame* pDecodedFrame = av_frame_alloc();
        ASSERT_NONZERO(pDecodedFrame)(codec);

        while (!frameFinished)
        {
            AVPacket* nextToBeDecodedPacket = 0;
            PacketPtr packet = getNextPacket();
            if (packet)
            {
                nextToBeDecodedPacket = packet->getPacket();

                // Whenever a packet STARTS a frame, avcodec_decode_video()
                // calls avcodec_get_buffer() to allocate a buffer.
                mVideoPacketPts = nextToBeDecodedPacket->pts;
            }
            else // No packet. End of file.
            {
                // Feed with 0 frames to extract cached frames
                // Note that it is allowed to call avcodec_decode_video2
                // with this 0 frame, even if the decoder does not have
                // caching.
                //
                // Paricularly, for decoding a png image adding one extra
                // 0 frame was required, after migrating to the 2014-jan-05
                // version of avcodec.
                nextToBeDecodedPacket = &nullPacket;
                mVideoPacketPts += boost::rational_cast<pts>(ticksPerFrame);
                nextToBeDecodedPacket->dts = mVideoPacketPts;
            }
            ASSERT_NONZERO(nextToBeDecodedPacket);

            int len1 = avcodec_decode_video2(codec, pDecodedFrame, &frameFinished, nextToBeDecodedPacket);
            if (len1 < 0)
            {
                LOG_WARNING << "Decode error: " << *this;
            }
            else
            {
                ASSERT_LESS_THAN_EQUALS(len1, nextToBeDecodedPacket->size);
                if (len1 < nextToBeDecodedPacket->size)
                {
                    ASSERT_LESS_THAN_EQUALS(nextToBeDecodedPacket->size - len1, 8);
                    // else: Packet probably made aligned?
                    // If the assert above fails,
                    // then see http://blog.tomaka17.com/2012/03/libavcodeclibavformat-tutorial/
                    // for the cases listed below 'The "isFrameAvailable" and "processedLength" variables are very important.'.
                    // One case is missing here (0 < len1 < packet.size)
                    // todo http://samples.ffmpeg.org
                }
            }

            if (frameFinished)
            {
                // DEBUG: saveDecodedFrame(codec,pDecodedFrame,size,frameFinished);

                // Using the pts value of the first 'sent' packet (instead of dts of latest packet)
                // works better (audio-video-sync) for codecs with a delay (example: MOV file with h.264)
                int64_t* storedPts = static_cast<int64_t*>(pDecodedFrame->opaque);
                if (storedPts != 0 &&
                    *storedPts != AV_NOPTS_VALUE)
                {
                    // Use PTS value of first packet sent into the decoder for this frame.
                    decodedFramePts = *storedPts;
                }
                else if (nextToBeDecodedPacket->dts != AV_NOPTS_VALUE)
                {
                    // Use DTS of latest packet sent into the decoder for this frame.
                    decodedFramePts = nextToBeDecodedPacket->dts;
                }
                else
                {
                    LOG_WARNING << "Could not deduce pts [" << *this << "]";
                }

                if (!frameTimeOk(decodedFramePts))
                {
                    // A whole frame was decoded, but it does not have the correct pts value. Get another.
                    frameFinished = 0;
                }
            }
            else
            {
                // len1  < 0 - error
                // len1 == 0 - end of file
                // len1  > 0 - valid data was decoded. Not the end yet.
                if (len1 < 0 ||
                    ((len1 == 0) && (getEOF())))
                {
                    // End of file reached. Signal this with null ptr.
                    av_frame_free(&pDecodedFrame);
                    mDeliveredFrame.reset();
                    static const std::string status("End of file");
                    VAR_DEBUG(status);
                    return mDeliveredFrame;
                }
            }
        }
        ASSERT_MORE_THAN_EQUALS_ZERO(pDecodedFrame->repeat_pict)(codec);
        if (pDecodedFrame->repeat_pict > 0)
        {
            NIY("Input video frame repeating is not supported yet");
        }

        if (parameters.getSkip())
        {
            // Output frame is not required, only advancement of position in file.
            // Note that decoding is required to determine proper frame pts values
            // (and thus determine proper advancenment of position in file).
            result = boost::make_shared<VideoSkipFrame>(parameters);
        }
        else
        {
            // Resample the frame (includes format conversion)
            AVFrame* pScaledFrame = av_frame_alloc();
            int bufferSize = avpicture_get_size(AV_PIX_FMT_RGB24, size.GetWidth(), size.GetHeight());
            boost::uint8_t * buffer = static_cast<boost::uint8_t*>(av_malloc(bufferSize * sizeof(uint8_t)));
            avpicture_fill(reinterpret_cast<AVPicture*>(pScaledFrame), buffer, AV_PIX_FMT_RGB24, size.GetWidth(), size.GetHeight());
            mSwsContext = sws_getCachedContext(mSwsContext,codec->width,
                codec->height,
                codec->pix_fmt,
                size.GetWidth(),
                size.GetHeight(),
                AV_PIX_FMT_RGB24,
                SWS_BICUBIC, 0, 0, 0);
            sws_scale(mSwsContext,pDecodedFrame->data,pDecodedFrame->linesize,0,codec->height,pScaledFrame->data,pScaledFrame->linesize);

            result =
                boost::make_shared<VideoFrame>(parameters,
                boost::make_shared<VideoFrameLayer>(
                boost::make_shared<wxImage>(wxImage(size, pScaledFrame->data[0], true).Copy())));

            av_freep(&buffer);
            av_frame_free(&pScaledFrame);
        }
        result->setPts(decodedFramePts);

        av_frame_free(&pDecodedFrame);
    }
    else
    {
        LOG_DEBUG << "Same frame again";
    }

    // DEBUG: saveScaledFrame(codec,size,result);
    ASSERT(result)(parameters)(codec)(decodedFramePts);
    if (!result->isA<VideoSkipFrame>())
    {
        ASSERT_EQUALS(result->getLayers().size(), 1);

        // Store the 'original time' for reference (testing)
        result->setTime(positionToFrameTime(result->getPts()));

	    // Clone the used frame. Must be done for multiple reasons.
	    // 1. See also "Same frame again" above
	    //    If the same frame is returned multiple times, any modifications on the frame
	    //    (for instance, pts value) will lead to a changed frame here. And the pts value
	    //    of the frame as used here is the pts value of the INPUT. When returned (by
	    //    Sequence) the pts value of the frame is overwritten with the pts OUTPUT value.
	    // 2. mDeliveredFrame may have already been queued somewhere (VideoDisplay, for example).
	    //    Changing mDeliveredFrame and returning that once more might thus change that previous frame also!
        mDeliveredFrame = make_cloned<VideoFrame>(result);
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize VideoFile::getSize()
{
    AVCodecContext* codec = getCodec();
    if (!codec)
    {
        // File could not be opened (deleted?)
        return  Properties::get().getVideoSize();
    }
    return wxSize(codec->width, codec->height);
}

FrameRate VideoFile::getFrameRate()
{
    AVRational frameRate = getStream()->r_frame_rate;
    return FrameRate(frameRate.num, frameRate.den);
}

uint64_t VideoFile::getVideoPacketPts()
{
    return mVideoPacketPts;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void VideoFile::startDecodingVideo(const VideoCompositionParameters& parameters)
{
    if (mDecodingVideo) return;

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.

    if (!canBeOpened()) { return; } // File could not be opened (deleted?)

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    mDecodingVideo = true;

    AVCodecContext* avctx = getCodec();
    avctx->opaque = this; // Store address to be able to access this object from the avcodec callbacks
    avctx->get_buffer2 = avcodec_get_buffer;

    AVCodec *videoCodec = avcodec_find_decoder(avctx->codec_id);
    ASSERT_NONZERO(videoCodec);

    avctx->workaround_bugs = 1; // Taken from ffplay.c
    avctx->error_concealment = 3; // Taken from ffplay.c

    if (!parameters.getOptimizeForQuality())
    {
        avctx->flags |= CODEC_FLAG_EMU_EDGE;
        avctx->flags2 |= CODEC_FLAG2_FAST;

        switch(getCodec()->codec_id)
        {
        case AV_CODEC_ID_H264:
            av_opt_set((void*)getCodec()->priv_data, "profile", "baseline", 0);
            av_opt_set((void*)getCodec()->priv_data, "preset", "ultrafast", 0);
            av_opt_set((void*)getCodec()->priv_data, "tune", "zerolatency,fastdecode", 0);
            av_opt_set((void*)getCodec()->priv_data, "x264opts", "rc-lookahead=0", 0);
            break;
        }
    }

    avctx->flags &= ~CODEC_FLAG_TRUNCATED; // Do not set this, causes bad frames

    int result = avcodec_open2(avctx, videoCodec, 0);
    ASSERT_MORE_THAN_EQUALS_ZERO(result)(avcodecErrorString(result));

    AVRational frameRate = getStream()->r_frame_rate;
    FrameRate videoFrameRate = FrameRate(frameRate.num, frameRate.den);
    if (videoFrameRate != Properties::get().getFrameRate())
    {
        LOG_DEBUG << "Frame rate conversion required from " << videoFrameRate << " to " << Properties::get().getFrameRate();
    }

    VAR_DEBUG(this)(getCodec());
}

void VideoFile::stopDecodingVideo()
{
    VAR_DEBUG(this);

    if (mDecodingVideo)
    {
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        avcodec_close(getCodec());
    }
    if (mSwsContext != 0)
    {
        sws_freeContext(mSwsContext);
        mSwsContext = 0;
    }

    mDecodingVideo = false;
}

//////////////////////////////////////////////////////////////////////////
// FROM FILE
//////////////////////////////////////////////////////////////////////////

bool VideoFile::useStream(const AVMediaType& type) const
{
    return (type == AVMEDIA_TYPE_VIDEO);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const VideoFile& obj)
{
    os  << static_cast<const File&>(obj) << '|'
        << obj.mDecodingVideo;
    return os;
}

void VideoFile::saveDecodedFrame(AVCodecContext* codec, AVFrame* frame, const wxSize& size, int frameFinished)
{
    static SwsContext* swsContext(0);
    static int count(0);

    AVFrame* pWriteToDiskFrame = av_frame_alloc();
    int bufferSizeToDisk = avpicture_get_size(AV_PIX_FMT_RGB24, codec->width, codec->height);
    boost::uint8_t * bufferToDisk = static_cast<boost::uint8_t*>(av_malloc(bufferSizeToDisk * sizeof(uint8_t)));
    avpicture_fill(reinterpret_cast<AVPicture*>(pWriteToDiskFrame), bufferToDisk, AV_PIX_FMT_RGB24, codec->width, codec->height);
    swsContext = sws_getCachedContext(swsContext,codec->width,
        codec->height,
        codec->pix_fmt,
        codec->width,
        codec->height,
        AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX | SWS_CPU_CAPS_MMX2, 0, 0, 0);
    sws_scale(swsContext,frame->data,frame->linesize,0,codec->height,pWriteToDiskFrame->data,pWriteToDiskFrame->linesize);

    wxImagePtr image = boost::make_shared<wxImage>(wxImage(wxSize(codec->width, codec->height), pWriteToDiskFrame->data[0], true).Copy());
    wxString filename;
    count++;
    VAR_ERROR(this)(count)(mPosition);
    filename
        << "D:\\savedframes\\" << getPath().GetName() << '_'
        << count << '_'
        << "original_"
        << codec->width << 'x' << codec->height << '_'
        << size.x << 'x' << size.y << '_' << frameFinished << ".jpg";
    image->SaveFile(filename);
    av_freep(&bufferToDisk);
    av_frame_free(&pWriteToDiskFrame);
}

void VideoFile::saveScaledFrame(AVCodecContext* codec, const wxSize& size, VideoFramePtr frame)
{
    static int count(0);
    wxString filename;
    filename
        << "D:\\savedframes\\" << getPath().GetName() << '_'
        << count++ << '_'
        << "delivered_"
        << codec->width << 'x' << codec->height << '_'
        << size.x << 'x' << size.y << ".jpg";
    mDeliveredFrame->getImage()->SaveFile(filename);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoFile::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(File);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IVideo);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoFile::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoFile::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoFile)
