#include "VideoFile.h"

#include "Constants.h"
#include "Convert.h"
#include "Node.h"
#include "Properties.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model {

static int const sMaxBufferSize = 10;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFile::VideoFile()
:	File()
,   mDecodingVideo(false)
,   mPosition(0)
,   mDeliveredFrame()
,   mDeliveredFrameInputPts(0)
,   mDeliveredFrameParameters()
{
    VAR_DEBUG(*this);
}

VideoFile::VideoFile(wxFileName path)
:	File(path,sMaxBufferSize)
,   mDecodingVideo(false)
,   mPosition(0)
,   mDeliveredFrame()
,   mDeliveredFrameInputPts(0)
,   mDeliveredFrameParameters()
{
    VAR_DEBUG(*this);
}

VideoFile::VideoFile(const VideoFile& other)
:   File(other)
,   mDecodingVideo(false)
,   mPosition(0)
,   mDeliveredFrame()
,   mDeliveredFrameInputPts(0)
,   mDeliveredFrameParameters()
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
    stopDecodingVideo();
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void VideoFile::moveTo(pts position)
{
    mDeliveredFrame.reset();
    mDeliveredFrameInputPts = 0;
    mDeliveredFrameParameters.reset();

    mPosition = position;

    AVCodecContext* codec = getCodec();
    pts positionBefore = position;
    switch (codec->codec_id)
    {
    case CODEC_ID_MPEG4:
        case CODEC_ID_H264:
            positionBefore -= 20; // Move back a bit to ensure that the resulting position <= required position, sometimes the stream is positioned on a keyframe AFTER the required frame
            break;
    }

    VAR_INFO(mPosition)(positionBefore);
    File::moveTo(positionBefore); // NOTE: This uses the pts in 'project' timebase units
}

void VideoFile::clean()
{
    VAR_DEBUG(this);

    stopDecodingVideo();

    mDeliveredFrame.reset();
    mDeliveredFrameInputPts = 0;
    mPosition = 0;

    File::clean();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoFile::getNextVideo(const VideoCompositionParameters& parameters)
{
    startDecodingVideo(parameters);

    AVPacket nullPacket;
    nullPacket.data = 0;
    nullPacket.size = 0;

    auto div = [this](boost::rational<int> num, boost::rational<int> divisor) -> int
    {
        return boost::rational_cast<int>(num / divisor);
    };

    auto modulo = [this,div](boost::rational<int> num, boost::rational<int> divisor) -> boost::rational<int>
    {
        num -= divisor * div(num,divisor);
        return num;
    };

    auto projectPositionToTimeInS = [this](pts position) -> boost::rational<int>
    {
        return boost::rational<int>(position) / Properties::get().getFrameRate();
    };

    auto timeToNearestInputFramesPts = [this, modulo](boost::rational<int> time) -> std::pair<int,int>
    {
        FrameRate fr = FrameRate(getStream()->r_frame_rate); // 24000/1001
        FrameRate timebase = FrameRate(getStream()->time_base); // 1/240000
        boost::rational<int> ticksPerFrame = 1 / (fr * timebase);

        boost::rational<int> firstFrame = time * fr;// * timebase;
        boost::rational<int> requiredStreamPts = firstFrame * ticksPerFrame;

        boost::rational<int> first = requiredStreamPts - modulo(requiredStreamPts,ticksPerFrame);
        boost::rational<int> second = first + ticksPerFrame;

        return std::make_pair(boost::rational_cast<int>(first),boost::rational_cast<int>(second));
    };

    // 'Resample' the frame timebase
    // Determine which pts value is required. This is required to first determine
    // if the previously returned frame should be returned again
    // \todo instead of duplicating frames, nicely take the two input frames 'around' the
    // required output pts time and 'interpolate' given these two frames time offsets with the required pts
    std::pair<int,int> requiredInputFrames = timeToNearestInputFramesPts(projectPositionToTimeInS(mPosition));
    pts requiredInputPts = requiredInputFrames.first;

    VAR_DEBUG(this)(requiredInputPts)(mDeliveredFrame)(mDeliveredFrameInputPts)(mPosition);
    ASSERT(!mDeliveredFrame || requiredInputPts >= mDeliveredFrameInputPts)(requiredInputPts)(mDeliveredFrameInputPts);
    ASSERT(!mDeliveredFrameParameters || *mDeliveredFrameParameters == parameters)(*mDeliveredFrameParameters)(parameters); // Ensure that mDeliveredFrame had the same set of VideoCompositionParameters

    if (!mDeliveredFrame || requiredInputPts > mDeliveredFrameInputPts)
    {
        // Decode new frame
        bool firstPacket = true;
        int frameFinished = 0;
        AVFrame* pFrame = avcodec_alloc_frame();
        ASSERT_NONZERO(pFrame);

        while (!frameFinished )
        {
            AVPacket* nextToBeDecodedPacket = 0;
            PacketPtr packet = getNextPacket();
            if (packet)
            {
                nextToBeDecodedPacket = packet->getPacket();

                // From http://dranger.com/ffmpeg/tutorial05.html:
                // When we get a packet from av_read_frame(), it will contain the PTS and DTS values for the information inside that packet.
                // But what we really want is the PTS of our newly decoded raw frame, so we know when to display it. However, the frame we
                // get from avcodec_decode_video() gives us an AVFrame, which doesn't contain a useful PTS value.
                // (Warning: AVFrame does contain a pts variable, but this will not always contain what we want when we get a frame.)
                // However, ffmpeg reorders the packets so that the DTS of the packet being processed by avcodec_decode_video() will always
                // be the same as the PTS of the frame it returns. But, another warning: we won't always get this information, either.
                //
                // Not to worry, because there's another way to find out the PTS of a frame, and we can have our program reorder the packets
                // by itself. We save the PTS of the first packet of a frame: this will be the PTS of the finished frame. So when the stream
                // doesn't give us a DTS, we just use this saved PTS.
                // Of course, even then, we might not get a proper pts. We'll deal with that later.
                if (firstPacket)
                {
                    // Store this to at least have one value. If the ->dts value indicates a proper position that will be used instead.
                    mDeliveredFrameInputPts = nextToBeDecodedPacket->pts; // Note: ->pts is expressed in time_base - of the stream - units
                    firstPacket = false;
                    VAR_DEBUG(requiredInputPts)(mDeliveredFrame)(mDeliveredFrameInputPts);
                }
            }
            else if (getCodec()->codec->capabilities & CODEC_CAP_DELAY)
            {
                nextToBeDecodedPacket = &nullPacket; // feed with 0 frames to extract the cached frames
            }

            bool endOfFile = true;
            if (nextToBeDecodedPacket)
            {
                int len1 = avcodec_decode_video2(getCodec(), pFrame, &frameFinished, nextToBeDecodedPacket);
                ASSERT_MORE_THAN_EQUALS_ZERO(len1);
                if (len1 > 0)
                {
                    endOfFile = false;
                    mDeliveredFrameInputPts = av_frame_get_best_effort_timestamp(pFrame);
                    mDeliveredFrameParameters.reset(new VideoCompositionParameters(parameters));
                    VAR_DEBUG(mDeliveredFrameInputPts)(*mDeliveredFrameParameters);
                }
                // else: For codecs with CODEC_CAP_DELAY, (len1 == 0) indicates end of the decoding
            }

            if (endOfFile)
            {
                // End of file reached. Signal this with null ptr.
                av_free(pFrame);
                mDeliveredFrame.reset();
                mDeliveredFrameInputPts = 0;
                static const std::string status("End of file");
                VAR_DEBUG(status);
                VAR_VIDEO(this)(status)(mPosition)(requiredInputPts);
                return mDeliveredFrame;
            }

            // If !mDeliveredFrame: first getNextVideo after object creation, or directly after a move.
            if (frameFinished && mDeliveredFrameInputPts < requiredInputPts)
            {
                VAR_DEBUG(requiredInputPts)(mDeliveredFrameInputPts);
                // A whole frame was decoded, but it does not have the correct pts value. Get another.
                frameFinished = 0;
            }

        }
        ASSERT_MORE_THAN_EQUALS_ZERO(pFrame->repeat_pict);
        if (pFrame->repeat_pict > 0)
        {
            NIY(_("Video frame repeating is not supported yet"));
        }

        static const int sMinimumFrameSize = 10;        // I had issues when generating smaller bitmaps. To avoid these, always
        wxSize size(parameters.getBoundingBox());
        size.x = std::max(size.x,sMinimumFrameSize);    // use a minimum framesize. The region of interest in videoclips will ensure
        size.y = std::max(size.y,sMinimumFrameSize);    // that any excess data is cut off.
        mDeliveredFrame = boost::make_shared<VideoFrame>(size, mPosition, pFrame->repeat_pict + 1);

        // Resample the frame size
        SwsContext* ctx = sws_getContext(
            getCodec()->width,
            getCodec()->height,
            getCodec()->pix_fmt,
            size.GetWidth(),
            size.GetHeight(),
            PIX_FMT_RGB24,
            SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX | SWS_CPU_CAPS_MMX2, 0, 0, 0);
        sws_scale(ctx,pFrame->data,pFrame->linesize,0,getCodec()->height,mDeliveredFrame->getData(),mDeliveredFrame->getLineSizes());
        sws_freeContext(ctx);

        av_free(pFrame);
    }
    else
    {
        LOG_DEBUG << "Same frame again";
    }

    ASSERT(mDeliveredFrame);
    mPosition += mDeliveredFrame->getRepeat();

    VAR_DEBUG(this)(mPosition)(requiredInputPts)(mDeliveredFrame)(mDeliveredFrameInputPts);
    return mDeliveredFrame;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize VideoFile::getSize()
{
    return wxSize(getCodec()->width, getCodec()->height);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void VideoFile::startDecodingVideo(const VideoCompositionParameters& parameters)
{
    if (mDecodingVideo) return;

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingVideo = true;

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    AVCodec *videoCodec = avcodec_find_decoder(getCodec()->codec_id);
    ASSERT_NONZERO(videoCodec);

    int result = avcodec_open(getCodec(), videoCodec);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    if (!parameters.getOptimizeForQuality())
    {
        switch(getCodec()->codec_id)
        {
        case CODEC_ID_H264:
            av_opt_set((void*)getCodec()->priv_data, "profile", "baseline", 0);
            av_opt_set((void*)getCodec()->priv_data, "preset", "ultrafast", 0);
            av_opt_set((void*)getCodec()->priv_data, "tune", "zerolatency,fastdecode", 0);
            av_opt_set((void*)getCodec()->priv_data, "x264opts", "rc-lookahead=0", 0);
            break;
        }
    }

    FrameRate videoFrameRate = FrameRate(getStream()->r_frame_rate);
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
    mDecodingVideo = false;
}

//////////////////////////////////////////////////////////////////////////
// FROM FILE
//////////////////////////////////////////////////////////////////////////

bool VideoFile::useStream(AVMediaType type) const
{
    return (type == AVMEDIA_TYPE_VIDEO);
}

void VideoFile::flush()
{
    if (mDecodingVideo)
    {
        avcodec_flush_buffers(getCodec());
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoFile& obj )
{
    os << static_cast<const File&>(obj) << '|' << obj.mDecodingVideo << '|' << obj.mPosition << '|';
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoFile::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<File>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace