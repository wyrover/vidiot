// Copyright 2013,2014 Eric Raijmakers.
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

#include "VideoFile.h"

#include "Constants.h"
#include "Convert.h"
#include "Node.h"
#include "Properties.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogBoost.h"
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

    startReadingPackets(); // To ensure that openFile is called.
    if (fileOpenFailed()) { return; } // File could not be opened (deleted?)

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

    if (fileOpenFailed())
    {
        // File could not be opened (deleted?)
        wxImagePtr compositeImage(boost::make_shared<wxImage>(parameters.getBoundingBox()));
        wxGraphicsContext* gc = wxGraphicsContext::Create(*compositeImage);
        wxString error_message1 = _("Missing file: ");
        wxString error_message2 = getPath().GetFullName();
        wxFont errorFont = wxFont(wxSize(0,20), wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        gc->SetFont(errorFont, wxColour(255,255,255));
        wxDouble w, h, d, e;
        gc->GetTextExtent(error_message1, &w, &h, &d, &e);
        gc->DrawText(error_message1, 20, 20);
        gc->DrawText(error_message2, 20, 20 + h );
        delete gc;
        return boost::make_shared<VideoFrame>(parameters, boost::make_shared<VideoFrameLayer>(compositeImage));
    }

    AVPacket nullPacket;
    nullPacket.data = 0;
    nullPacket.size = 0;
    //nullPacket.buf = 0; // for new version of avcodec
    //ASSERT_ZERO(codec->refcounted_frames); // for new version of avcodec, see avcodec_decode_video2 docs

    auto div = [this](rational64 num, rational64 divisor) -> int64_t
    {
        return boost::rational_cast<int64_t>(num / divisor);
    };

    auto modulo = [this,div](rational64 num, rational64 divisor) -> rational64
    {
        num -= divisor * div(num,divisor);
        return num;
    };

    auto projectPositionToTimeInS = [this](pts position) -> rational64
    {
        return rational64(position) / Properties::get().getFrameRate();
    };

    auto timeToNearestInputFramesPts = [this, modulo](rational64 time) -> boost::tuple<pts,pts,pts>
    {
        FrameRate fr = FrameRate(getStream()->r_frame_rate); // 24000/1001
        FrameRate timebase = FrameRate(getStream()->time_base); // 1/240000
        rational64 ticksPerFrame = 1 / (fr * timebase);

        rational64 firstFrame = time * fr;// * timebase;
        rational64 requiredStreamPts = firstFrame * ticksPerFrame;

        rational64 first = requiredStreamPts - modulo(requiredStreamPts,ticksPerFrame);
        rational64 second = first + ticksPerFrame;

        return boost::make_tuple(boost::rational_cast<pts>(first),boost::rational_cast<pts>(requiredStreamPts),boost::rational_cast<pts>(second));
    };

    // 'Resample' the frame timebase
    // Determine which pts value is required. This is required to first determine
    // if the previously returned frame should be returned again
    // \todo instead of duplicating frames, nicely take the two input frames 'around' the
    // required output pts time and 'interpolate' given these two frames time offsets with the required pts
    boost::tuple<pts,pts,pts> requiredInputFrames = timeToNearestInputFramesPts(projectPositionToTimeInS(mPosition));
    pts requiredInputPts = requiredInputFrames.get<1>();

    AVCodecContext* codec = getCodec();

    VAR_DEBUG(this)(requiredInputFrames)(requiredInputPts)(mDeliveredFrame)(mDeliveredFrameInputPts)(mPosition);
    // NOT:
    //ASSERT(!mDeliveredFrame || requiredInputPts >= mDeliveredFrameInputPts)(requiredInputPts)(mDeliveredFrameInputPts)(codec);
    // Sometimes the gotten input frame covers two output frames. Subsequently, getting the next video frame triggers the assert.
    // Typically seen with H264 - MPEG4AVC (part10) (avc1) files with a frame rate of 28 frames/s.
    // NOT:
    //ASSERT(!mDeliveredFrameParameters || *mDeliveredFrameParameters == parameters)(*mDeliveredFrameParameters)(parameters)(codec); // Ensure that mDeliveredFrame had the same set of VideoCompositionParameters
    // Parameters can be changed during playback by - for instance - changing 'showBoundingBox'

    if (!mDeliveredFrame || requiredInputPts > mDeliveredFrameInputPts)
    {
        // Decode new frame
        bool firstPacket = true;
        int frameFinished = 0;
        AVFrame* pDecodedFrame = avcodec_alloc_frame(); // for new version of avcodec : av_frame_alloc();
        ASSERT_NONZERO(pDecodedFrame)(codec);

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
                int len1 = avcodec_decode_video2(getCodec(), pDecodedFrame, &frameFinished, nextToBeDecodedPacket);
                ASSERT_MORE_THAN_EQUALS_ZERO(len1)(codec);

                if (len1 > 0)
                {
                    endOfFile = false;
                    mDeliveredFrameInputPts = av_frame_get_best_effort_timestamp(pDecodedFrame);
                    if (mDeliveredFrameInputPts == AV_NOPTS_VALUE)
                    {
                        AVStream* stream = getStream();
                        ASSERT(stream)(codec);
                        mDeliveredFrameInputPts = av_q2d(stream->time_base) * pDecodedFrame->pts;
                    }
                    ASSERT_DIFFERS(mDeliveredFrameInputPts, AV_NOPTS_VALUE)(codec);
                    mDeliveredFrameParameters.reset(new VideoCompositionParameters(parameters));
                    VAR_DEBUG(mDeliveredFrameInputPts)(*mDeliveredFrameParameters);
                }
                // else: For codecs with CODEC_CAP_DELAY, (len1 == 0) indicates end of the decoding
            }

            if (endOfFile)
            {
                // End of file reached. Signal this with null ptr.
                av_free(pDecodedFrame);
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
        ASSERT_MORE_THAN_EQUALS_ZERO(pDecodedFrame->repeat_pict)(codec);
        if (pDecodedFrame->repeat_pict > 0)
        {
            NIY(_("Input video frame repeating is not supported yet"));
        }

        static const int sMinimumFrameSize = 10;        // I had issues when generating smaller bitmaps. To avoid these, always
        wxSize size(parameters.getBoundingBox());
        size.x = std::max(size.x,sMinimumFrameSize);    // use a minimum framesize. The region of interest in videoclips will ensure
        size.y = std::max(size.y,sMinimumFrameSize);    // that any excess data is cut off.

        // Create temp data holder for the frame conversion
        AVFrame* pScaledFrame = avcodec_alloc_frame();
        int bufferSize = avpicture_get_size(PIX_FMT_RGB24, size.GetWidth(), size.GetHeight());
        boost::uint8_t * buffer = static_cast<boost::uint8_t*>(av_malloc(bufferSize * sizeof(uint8_t)));
        // Assign appropriate parts of buffer to image planes in pScaledFrame
        avpicture_fill(reinterpret_cast<AVPicture*>(pScaledFrame), buffer, PIX_FMT_RGB24, size.GetWidth(), size.GetHeight());

        // Resample the frame (includes format conversion)
        SwsContext* ctx = sws_getContext(
            getCodec()->width,
            getCodec()->height,
            getCodec()->pix_fmt,
            size.GetWidth(),
            size.GetHeight(),
            PIX_FMT_RGB24,
            SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX | SWS_CPU_CAPS_MMX2, 0, 0, 0);
        sws_scale(ctx,pDecodedFrame->data,pDecodedFrame->linesize,0,getCodec()->height,pScaledFrame->data,pScaledFrame->linesize);
        sws_freeContext(ctx);

        mDeliveredFrame =
            boost::make_shared<VideoFrame>(parameters,
            boost::make_shared<VideoFrameLayer>(
            boost::make_shared<wxImage>(wxImage(size, pScaledFrame->data[0], true).Copy())));

         av_free(buffer);
         av_free(pDecodedFrame);
         av_free(pScaledFrame);
    }
    else
    {
        LOG_DEBUG << "Same frame again";

        // VideoFrame must be cloned, frame repeating is not supported. If a frame is to be output multiple
        // times, avoid pts calculation problems by making multiple unique frames.
        //
        // Furthermore, note that mDeliveredFrame may have already been queued somewhere (VideoDisplay, for example).
        // Changing mDeliveredFrame and returning that once more might thus change that previous frame also!
        mDeliveredFrame = make_cloned<VideoFrame>(mDeliveredFrame);
    }

    ASSERT(mDeliveredFrame)(parameters)(codec);
    mPosition++;

    VAR_DEBUG(this)(mPosition)(requiredInputPts)(mDeliveredFrame)(mDeliveredFrameInputPts);
    return mDeliveredFrame;
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

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void VideoFile::startDecodingVideo(const VideoCompositionParameters& parameters)
{
    if (mDecodingVideo) return;

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.

    if (fileOpenFailed()) { return; } // File could not be opened (deleted?)

    mDecodingVideo = true;

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    AVCodec *videoCodec = avcodec_find_decoder(getCodec()->codec_id);
    ASSERT_NONZERO(videoCodec);

    int result = avcodec_open2(getCodec(), videoCodec, 0);
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
    os  << static_cast<const File&>(obj) << '|'
        << obj.mDecodingVideo << '|'
        << obj.mPosition << '|';
    return os;
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
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoFile::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoFile::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoFile)