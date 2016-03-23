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

#include "File.h"

#include "AutoFolder.h"
#include "Config.h"
#include "Convert.h"
#include "Dialog.h"
#include "FileMetaDataCache.h"
#include "FilePacket.h"
#include "Project.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilInitAvcodec.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilThread.h"
#include "WximageFile.h"

namespace model {

IMPLEMENTENUM(FileType);

// static
const int File::STREAMINDEX_UNDEFINED = -1;
// Set to maxint deliberately. In some exceptional cases (file removed from disk),
// this value may actually be used in certain computations. To avoid asserts use
// a 'not too small' value. What typically can go wrong is Transition::make*Clip()
// which uses adjustBegin/End to create the clip. With a too small length for an
// unavailable file, crashes occur.
const int File::LENGTH_UNDEFINED = std::numeric_limits<int>::max();

// static
wxString File::sSupportedVideoExtensions{ "*.asf;*.avi;*.ogm;*.mov;*.mp4;*.mpeg;*.mpg;*.mkv;*.dv;*.gxf;*.m2t;*.m2ts;*.m2v;*.m4v;*.mts;*.3gp;*.3g2;*.asx;*.flv;*.f4v;*.ogv;*.webm;*.duk;*.dvr-ms;*.mv;*.pva;*.rm;*.rmvb;*.smv;*.ts;*.vob;*.wmv" };
// static
wxString File::sSupportedAudioExtensions{ "*.wav;*.mp3;*.flac;*.m2a;*.m4a;*.8svx;*.aa3;*.aac;*.aacp;*.ac3;*.act;*.aif;*.aiff;*.amr;*.ape;*.au;*.caf;*.dts;*.mid;*.mka;*.mp1;*.mpc;*.mpp;*.mp+;*.ogg;*.oma;*.qcp;*.rso;*.tta;*.voc;*.vqf;*.wma;*.xwma" };
//static
wxString File::sSupportedImageExtensions{ "*.bmp;*.gif;*.jpg;*.png;*.tga;*.tif;*.tiff" };

pts getFrameCount(AVStream* stream, int64_t duration)
{
    ASSERT_DIFFERS(duration, AV_NOPTS_VALUE);
    if (duration == 1)
    {
        // Stil image: disregard the timebase, since rounding errors
        // (timebase of file different than project time base)
        // may result in the outcome '0'.
        return 1;
    }
    return Convert::rationaltimeToPts(rational64(sSecond, 1) * rational64(duration, 1) * rational64(stream->time_base.num, stream->time_base.den));
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

File::File()
    : IFile{}
    , Node{}
    // Attributes
    , mPath()
    , mName()
    , mNumberOfFrames(boost::none)
    // Buffering
    , mMaxBufferSize{ 0 }
    , mPackets{ 1 }
{
    VAR_DEBUG(this);
}

File::File(const wxFileName& path, int buffersize)
    : IFile{}
    , Node{}
    // Attributes
    , mPath(path)
    , mName()
    , mNumberOfFrames(boost::none)
    // Buffering
    , mMaxBufferSize(buffersize)
    , mPackets(1)
{
    VAR_DEBUG(this);
    readMetaData();
}

File::File(const File& other)
    : IFile{}
    , Node{}
    // Attributes
    , mPath{ other.mPath }
    , mName{ other.mName }
    , mNumberOfFrames{ other.mNumberOfFrames }
    , mHasVideo{ other.mHasVideo }
    , mHasAudio{ other.mHasAudio }
    , mMaximumStartPts{ other.mMaximumStartPts }
    // Status of opening
    , mMetaDataKnown{ other.mMetaDataKnown }
    , mFileOpenedOk{ other.mFileOpenedOk }
    // Buffering
    , mMaxBufferSize{ other.mMaxBufferSize }
    , mPackets{ 1 }
{
    VAR_DEBUG(this);
}

File* File::clone() const
{
    return new File(static_cast<const File&>(*this));
}

void File::onCloned()
{
}

File::~File()
{
    VAR_DEBUG(this);
    clean();
}

//////////////////////////////////////////////////////////////////////////
// INODE
//////////////////////////////////////////////////////////////////////////

NodePtrs File::findPath(const wxString& path)
{
    NodePtrs result;
    if (util::path::equals(mPath,path))
    {
        result.push_back(self());
    }
    return result;
}

bool File::mustBeWatched(const wxString& path)
{
    if (util::path::isParentOf(path,mPath))
    {
        // Yes, is parent folder of this file
        return true;
    }
    return false;
}

void File::check(bool immediately)
{
    model::NodePtr parent = getParent();
    if (parent)
    {
        if (parent->isA<model::AutoFolder>())
        {
            // updated via autofolder indexing
            boost::dynamic_pointer_cast<AutoFolder>(parent)->check();
        }
        else
        {
            mMetaDataKnown = false;
            readMetaData();
            if (!canBeOpened())
            {
                gui::Dialog::get().getConfirmation(_("File removed"), wxString::Format(_("The file %s has been removed from disk. File is removed from project also."), util::path::toPath(mPath)));
                parent->removeChild(self());
            }
        }
    }
    else
    {
        // File is not part of the project view. For exception handling during playback, the file
        // will return empty video/audio data instead of showing a dialog.
    }
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts File::getLength() const
{
    // Cached locally in mNumberOfFrames to avoid continous locking in the filemetadatacache.
    if (!mNumberOfFrames)
    {
        boost::optional<pts> length{ FileMetaDataCache::get().getLength(getPath()) };
        if (!length)
        {
            File clone(*this);
            clone.readMetaData();
            if (clone.canBeOpened())
            {
                mNumberOfFrames.reset(clone.getNumberOfFrames());
                FileMetaDataCache::get().setLength(getPath(), *mNumberOfFrames);
            }
            else
            {
                VAR_WARNING(*this);
                mNumberOfFrames.reset(LENGTH_UNDEFINED);
            }
        }
        else
        {
            mNumberOfFrames = length; // Cached locally to avoid continous locking in the filemetadatacache.
        }
    }
    ASSERT(mNumberOfFrames);
    pts result{ *mNumberOfFrames };
    return *mNumberOfFrames;
}

void File::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
    ASSERT_MORE_THAN_EQUALS_ZERO(position);
    if (position == 0)
    {
        // No seek required, just re-open file. Furthermore, for some files seeking to '0' causes problems
        // whereas directly getting packets from these files results in proper decoding.
        // Maybe these files do not start with a keyframe, causing the problems?
        stopReadingPackets();
        closeFile();
    }

    openFile(); // Needed for avcodec calls below

    if (!canBeOpened()) { return; } // File probably closed

    stopReadingPackets();

    int64_t timestamp = model::Convert::ptsToMicroseconds(position);
    int flags{ mFileContext->flags };
    if ((mFileContext->duration != AV_NOPTS_VALUE && timestamp >= mFileContext->duration) ||
        (position >= mNumberOfFrames))
    {
        // Can happen when changing a clip's speed.
        // The preview of the clip is positioned 'in the center' of the clip.
        // That position may be beyond the file length when the speed factor
        // is high.
        mEOF = true;
    }
    else
    {
        ASSERT_MORE_THAN_EQUALS_ZERO(timestamp);
        VAR_DEBUG(timestamp)(mFileContext->duration);

        if (timestamp > 0)
        {
            // Do not seek if == 0, may mess up some files. See remark at top of method.

            if (mFileContext->start_time != AV_NOPTS_VALUE)
            {
                timestamp += mFileContext->start_time;
            }

            // First, try seeking to a keyframe at the given position.
            int result = 0;
            result = avformat_seek_file(mFileContext, -1, std::numeric_limits<int64_t>::min(), timestamp, std::numeric_limits<int64_t>::max(), 0);
            if (result < 0)
            {
                // Second, try seeking to a keyframe before the given position.
                result = avformat_seek_file(mFileContext, -1, std::numeric_limits<int64_t>::min(), timestamp, std::numeric_limits<int64_t>::max(), AVSEEK_FLAG_BACKWARD);
                if (result < 0)
                {
                    // Last resort, any frame will do.
                    result = avformat_seek_file(mFileContext, -1, std::numeric_limits<int64_t>::min(), timestamp, std::numeric_limits<int64_t>::max(), AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
                    LOG_WARNING << "Seek to " << position << " resulted in non-keyframe position and result " << result << " for file " << *this;
                    if (result < 0)
                    {
                        // Extracting data from an improperly initialized file will cause a crash.
                        mFileOpenedOk = false;
                    }
                }
            }
            if (mFileOpenedOk)
            {
                ASSERT_MORE_THAN_EQUALS_ZERO(result)(avcodecErrorString(result))(*this);
            }
            else
            {
                // Error. Do not try again, but trigger reopening the file to start at the beginning.
                VAR_ERROR(mFileOpenedOk);
                stopReadingPackets();
                closeFile();
            }
        }
    }

    ASSERT_ZERO(mPackets.getSize());
    mPackets.resize(1); // Ensures that only one packet is buffered (used for thumbnail generation).
    mTwoInARow = 0;

    VAR_DEBUG(this);
}

wxString File::getDescription() const
{
    return util::path::toName(mPath);
}

void File::clean()
{
    VAR_DEBUG(this);
    stopReadingPackets();
    closeFile();
}

//////////////////////////////////////////////////////////////////////////
// IPATH
//////////////////////////////////////////////////////////////////////////

wxFileName File::getPath() const
{
    return mPath;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxString File::getName() const
{
    if (hasParent() && getParent()->isA<AutoFolder>())
    {
        return util::path::toName(mPath);
    }
    return mPath.GetLongPath();
};

void File::readMetaData()
{
    if (mMetaDataKnown) { return; }
    openFile();
    closeFile();
}

bool File::canBeOpened()
{
    readMetaData();
    return mFileOpenedOk;
}

bool File::hasVideo()
{
    ASSERT(mMetaDataKnown)(this);
    return mHasVideo;
}

bool File::hasAudio()
{
    ASSERT(mMetaDataKnown)(this);
    return mHasAudio;
}

std::map<const wxString, FileType> makeMap()
{
    std::map<const wxString, FileType> result;
    std::string part;

    wxStringTokenizer tokenizerAudio(File::sSupportedAudioExtensions, ';');
    while (tokenizerAudio.HasMoreTokens())
    {
        wxString extension{ tokenizerAudio.GetNextToken() };
        extension.erase(0, 2);
        result[extension] = FileType_Audio;
    }

    wxStringTokenizer tokenizerImage(File::sSupportedImageExtensions, ';');
    while (tokenizerImage.HasMoreTokens())
    {
        wxString extension{ tokenizerImage.GetNextToken() };
        extension.erase(0, 2);
        result[extension] = FileType_Image;
    }

    wxStringTokenizer tokenizerVideo(File::sSupportedVideoExtensions, ';');
    while (tokenizerVideo.HasMoreTokens())
    {
        wxString extension{ tokenizerVideo.GetNextToken() };
        extension.erase(0, 2);
        result[extension] = FileType_Video;
    }

    result["png"] = FileType_Title;
    result["tiff"] = FileType_Title;
    result["tif"] = FileType_Title;
    return result;
}

FileType File::getType() const
{
    static std::map<const wxString, FileType> sMap{ makeMap() };
    auto it = sMap.find(mPath.GetExt());
    if (it != sMap.end())
    {
        return it->second;
    }
    return FileType_Video; // todo add filetype unknown + warning here?
}


//////////////////////////////////////////////////////////////////////////
// STREAMS INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

bool File::useStream(const AVMediaType& type) const
{
    return false;
}

AVStream* File::getStream()
{
    openFile();
    if (canBeOpened() && mFileContext && mStreamIndex != STREAMINDEX_UNDEFINED)
    {
        return mFileContext->streams[mStreamIndex];
    }
    return 0;
}

int64_t File::getStreamStartPosition()
{
    return mMaximumStartPts;
}

//////////////////////////////////////////////////////////////////////////
// PACKETS INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

void File::startReadingPackets()
{
    // If the end of file is reached, a subsequent getNext* should not
    // trigger a new (useless) sequence of startReadingPackets,
    // bufferPacketsThread, "bufferPacketsThread: End of file."
    // (and this, over and over again....).
    //
    // First a moveTo() is required to reset EOF.
    if (getEOF()) return;

    openFile();
    if (!canBeOpened()) { return; } // File probably closed

    if (mReadingPackets) return;

    VAR_DEBUG(this);

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    mReadingPackets = true;
    ASSERT(mBufferPacketsThreadPtr == nullptr); // To avoid leaking threads
    try
    {
        mBufferPacketsThreadPtr.reset(new boost::thread(std::bind(&File::bufferPacketsThread,this)));
    }
    catch (boost::exception &e)
    {
        FATAL(boost::diagnostic_information(e));
    }

    VAR_DEBUG(this);
}

void File::stopReadingPackets()
{
    VAR_DEBUG(this)(mReadingPackets)(mEOF);
    if (!mReadingPackets && !mEOF) return; // !mEOF is needed since we still want the buffers to be cleared, in the case that bufferPacketsThread has already delivered the last packet

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    // Avoid new packets to be retrieved from the file in bufferPacketsThread.
    mReadingPackets = false;

    // Clear fifo to unblock any pending push in bufferPacketsThread.
    mPackets.flush();

    // Wait until thread ends.
    if (mBufferPacketsThreadPtr)
    {
        mBufferPacketsThreadPtr->join();
        mBufferPacketsThreadPtr.reset();
    }

    // When this lock is taken, it is certain that no 'pop' is
    // blocking the return of getNextPacket().
    boost::mutex::scoped_lock stoplock(sMutexStop);

    // Clear buffers again, since one new (empty) packet may have
    // been added when ending bufferPacketsThread. This is done
    // AFTER taking the lock sMutexStop to avoid clearing the
    // '0' packet before getNextPacket() had a chance to return.
    mPackets.flush();

    // Flush any buffered data
    // Typically, flush avcodec decoding buffers in AudioFile and VideoFile.
    // The remaining avcodec buffers are no longer necessary.
    if (avcodec_is_open(getCodec()))
    {
        avcodec_flush_buffers(getCodec());
    }

    // From this point onwards, startReadingPackets should initalize reading
    // again.
    mEOF = false;

    VAR_DEBUG(this);
}

AVCodecContext* File::getCodec()
{
    openFile();
    if (!canBeOpened()) { return 0; } // File probably closed

    ASSERT(mFileContext->streams[mStreamIndex]);
    return mFileContext->streams[mStreamIndex]->codec;
}

PacketPtr File::getNextPacket()
{
    if (mEOF)
    {
        // After EOF is reached, first a 'moveTo' must be done.
        // Since in bufferPacketsThread() mReadingPackets is set to
        // false also, we need to do this, to avoid restarting the
        // bufferPacketsThread() in method startReadingPackets()

        VAR_DEBUG(mEOF);

        if (mPackets.getSize() == 0)
        {
            // EOF was reached AND the list of remaining packets
            // is empty. Signal this.
            LOG_DEBUG << "EOF";
            return PacketPtr();
        }
    }
    else
    {
        startReadingPackets(); // This is the normal trigger for starting to read
    }

    // This lock is used to signal that an external thread is blocked on
    // getting a new packet.
    boost::mutex::scoped_lock stoplock(sMutexStop);

    if (++mTwoInARow == 2)
    {
        mPackets.resize(mMaxBufferSize);
    }
    PacketPtr packet = mPackets.pop();
    return packet;
}

bool File::getEOF() const
{
    return mEOF;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pts File::getNumberOfFrames() const
{
    ASSERT(mMetaDataKnown); // If not initialized, then all sorts of problems (calculation mistakes) can occur.
    ASSERT(mNumberOfFrames);
    return *mNumberOfFrames;
}

void File::openFile()
{
    if (mFileOpened) return;
    mFileOpened = true;
    VAR_DEBUG(this);

    mMetaDataKnown = true;
    mFileOpenedOk = false; // Is reset after the open succeeds

    int result = 0;
    wxString path = mPath.GetLongPath();

    if (getType() == FileType_Title && wxImage::CanRead(path))
    {
        // Use wxImage to read from this file
        mNumberOfFrames.reset(1);
        mHasVideo = true;
        mHasAudio = false;
        mFileOpenedOk = true;
        return;
    }

    {
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        result = avformat_open_input(&mFileContext, path, 0, 0);
    }

    if (result != 0)
    {
        // Some error occured when opening the file.
        VAR_WARNING(path)(result)(avcodecErrorString(result))(*this);
        return;
    }
    {
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        result = avformat_find_stream_info(mFileContext,0);
        if (result < 0) // Some error occured when reading stream info. Close the file again.
        {
            VAR_DEBUG(path)(result)(avcodecErrorString(result))(*this);
            avformat_close_input(&mFileContext); // Requires the lock also
            ASSERT_ZERO(mFileContext);
            return;
        }
    }

    auto setNumberOfFrames = [this](pts nFrames)
    {
        if ((nFrames != AV_NOPTS_VALUE) &&
            (nFrames != 0) &&
            (!mNumberOfFrames || *mNumberOfFrames < nFrames))
        {
            mNumberOfFrames.reset(nFrames);
            ASSERT(mNumberOfFrames);
            ASSERT_MORE_THAN_EQUALS_ZERO(*mNumberOfFrames);
        }
    };

    auto isAudioSupported = [this,path](AVStream* stream) -> bool
    {
        if (stream->codec->codec_type != AVMEDIA_TYPE_AUDIO)
        {
            return false;
        }
        VAR_DEBUG(stream->codec->sample_fmt)(stream->codec->channels)(stream->codec->sample_rate);
        if (stream->codec->channels <= 0)
        {
            LOG_WARNING << "Unsupported audio stream '" << path << "'. Number of channels is " << stream->codec->channels << ".";
            return false;
        }
        if (stream->codec->sample_fmt == AV_SAMPLE_FMT_NONE)
        {
            LOG_WARNING << "Unsupported audio stream '" << path << "'. Sample format is unknown.";
            return false;
        }
        if (av_get_bytes_per_sample(stream->codec->sample_fmt) == 0)
        {
            LOG_WARNING << "Unsupported audio stream '" << path << "'. Number of bytes per sample is unknown.";
            return false;
        }
        if (stream->codec->sample_rate < 4000 || stream->codec->sample_rate > 256000)
        {
            LOG_WARNING << "Unsupported audio stream '" << path << "'. Sample rate (" << stream->codec->sample_rate << ") too big.";
            return false;
        }
        return true;
    };

    auto isVideoSupported = [this,path](AVStream* stream) -> bool
    {
        if (stream->codec->codec_type != AVMEDIA_TYPE_VIDEO)
        {
            return false;
        }
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC)
        {
            // This stream holds a (cover) image embedded in a (for example) mp3 file.
            LOG_WARNING << "Unsupported video stream '" << path << "' (embedded picture)";
            return false;
        }
        if (stream->codec->width == 0)
        {
            LOG_WARNING << "Unsupported video stream '" << path << "' (width 0)";
            return false;
        }
        if (stream->codec->height == 0)
        {
            LOG_WARNING << "Unsupported video stream '" << path << "' (height 0)";
            return false;
        }
        return true;
    };

    mNumberOfFrames = boost::none;
    mStreamIndex = STREAMINDEX_UNDEFINED;
    mMaximumStartPts = 0;
    for (unsigned int i=0; i < mFileContext->nb_streams; ++i)
    {
        AVStream* stream = mFileContext->streams[i];

        stream->discard = AVDISCARD_NONE;

        if (isVideoSupported(stream))
        {
            mHasVideo = true;

            if (stream->duration != AV_NOPTS_VALUE)
            {
                setNumberOfFrames(getFrameCount(stream, stream->duration));
            }
            if (stream->nb_frames != AV_NOPTS_VALUE)
            {
                // Convert to the Project frame rate.
                AVRational rate = av_stream_get_r_frame_rate(stream);
                setNumberOfFrames(Convert::timeToPts(Convert::ptsToTime(stream->nb_frames, FrameRate{ rate.num, rate.den })));
            }
            if (stream->start_time != AV_NOPTS_VALUE &&
                stream->start_time > mMaximumStartPts)
            {
                mMaximumStartPts = stream->start_time;
            }
            // todo BBC News_BBC TWO_2010_06_30_01_23_00.wtv has multiple audio streams
            // add a (optional) setting to 'details' for selecting the audio (and video) stream in case of multiple streams.
        }
        else if (isAudioSupported(stream))
        {
            mHasAudio = true;
            // For files without video, determine the number of 'virtual video frames'.
            if (stream->duration != AV_NOPTS_VALUE)
            {
                setNumberOfFrames(getFrameCount(stream, stream->duration));
            }
            if (stream->start_time != AV_NOPTS_VALUE &&
                stream->start_time > mMaximumStartPts)
            {
                mMaximumStartPts = stream->start_time;
            }
        }
        else
        {
            VAR_DEBUG(stream);
            stream->discard = AVDISCARD_ALL;
            continue; // To ensure that this stream is not used in case the video/audio contents is not supported
        }

        if ((mStreamIndex == STREAMINDEX_UNDEFINED) && useStream(stream->codec->codec_type))
        {
            mStreamIndex = i;
        }
        else
        {
            // NOT: stream->discard = AVDISCARD_ALL; -- if a file has both audio and video use the maximum length of these two streams
        }
    }

    if (!mNumberOfFrames &&
        mFileContext->nb_streams > 0)
    {
        VAR_WARNING(*this);
        gui::StatusBar::get().pushInfoText(wxString::Format(_("Scanning %s to determine media length."), mPath.GetFullName()));
        AVPacket pkt1 = { 0 };
        AVPacket* packet = &pkt1;
        std::vector<pts> streamPts(mFileContext->nb_streams, 0); // Note: no {, since that'll cause the wrong size
        std::vector<pts> streamPackets(mFileContext->nb_streams, 0);
        while (av_read_frame(mFileContext, packet) >= 0)
        {
            if (packet->pts != AV_NOPTS_VALUE) { streamPts[packet->stream_index] = std::max(streamPts[packet->stream_index], packet->pts); }
            if (isVideoSupported(mFileContext->streams[packet->stream_index])) { streamPackets[packet->stream_index]++; }
            av_packet_unref(packet);
        }
        // Reset position to beginning again. Otherwise, first playback (without 'moveTo' first) will cause errors.
        avformat_seek_file(mFileContext, -1, std::numeric_limits<int64_t>::min(), 0, std::numeric_limits<int64_t>::max(), 0);

        // todo store this info in the cache?
        if (std::any_of(streamPts.begin(), streamPts.end(), [](pts value) { return value > 0; }))
        {
            // Try to extract length data by looking at the pts values in the packets.
            std::vector<pts>::iterator it{ std::max_element(streamPts.begin(), streamPts.end()) };
            int index{ narrow_cast<int>(std::distance(streamPts.begin(), it)) };
            pts nFrames{ getFrameCount(mFileContext->streams[index], *it) };
            setNumberOfFrames(nFrames);
            VAR_WARNING(*this)(nFrames);
        }
        else
        {
            // Fallback: use number of packets. May be too much, but then the user can still cut off the end of the clip.
            pts nFrames{ *std::max_element(streamPackets.begin(), streamPackets.end()) };
            setNumberOfFrames(nFrames);
            VAR_WARNING(*this)(nFrames);
        }
    }

    if ((!mHasVideo && !mHasAudio) ||
        (!mNumberOfFrames) ||
        (*mNumberOfFrames <= 0)) // <= 0: Some files have streams, but with all lengths == 0 (once happened when indexing by mistake ffprobe.exe)
    {
        LOG_WARNING << "No correct stream found " << '(' << (*this) << ')';
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        avformat_close_input(&mFileContext);
        ASSERT_ZERO(mFileContext);
        mNumberOfFrames = boost::none;
        return;
    }
    if (mStreamIndex != STREAMINDEX_UNDEFINED)
    {
        AVStream* stream = mFileContext->streams[mStreamIndex];
        ASSERT_NONZERO(stream);
        AVCodecContext* codec = stream->codec;
        ASSERT_NONZERO(codec);
        VAR_DEBUG(stream)(codec);
    }
    VAR_DEBUG(mFileContext)(mStreamIndex)(mNumberOfFrames);
    mFileOpenedOk = true;
}

void File::closeFile()
{
    VAR_DEBUG(this);
    if (!mFileOpened) { return; }
    if (!canBeOpened()) { return; }

    boost::mutex::scoped_lock lock(Avcodec::sMutex);
    avformat_close_input(&mFileContext);
    ASSERT_ZERO(mFileContext);
    mFileOpened = false;
}

//////////////////////////////////////////////////////////////////////////
// THREADS
//////////////////////////////////////////////////////////////////////////

void File::bufferPacketsThread()
{
    util::thread::setCurrentThreadName("BufferPackets");
    VAR_DEBUG(this);

    AVPacket pkt1 = { 0 };
    AVPacket* packet = &pkt1;

    while (mReadingPackets)
    {
        if (av_read_frame(mFileContext, packet) < 0)
        {
            LOG_DEBUG << "End of file.";
            mEOF = true;
            mPackets.push(PacketPtr());
            break;
        }
        ASSERT_MORE_THAN_EQUALS_ZERO(packet->size);

        if (packet->stream_index == mStreamIndex)
        {
            PacketPtr p = boost::make_shared<Packet>(packet);
            mPackets.push(p);
        }
        av_packet_unref(packet);
    }
    if (!mReadingPackets)
    {
        // Reading packets was explicitly stopped (as opposed to reading until end of file).
        // Now we clear the packets fifo completely, then insert one empty packet.

        // One new packet may be added when a pending push was unblocked by the flush
        // in 'stopReadingPackets()'.
        mPackets.flush();

        // Insert one packet to signal the end of the packet stream. This unblocks
        // any pending 'getNextPacket()' by returning a '0' packet.
        mPackets.push(PacketPtr());
    }
    mReadingPackets = false; // Needed for any breaks above (implicit stop at end of file)

    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const File& obj)
{
    os  << &obj << '|'
        << obj.mPath << '|'
        << obj.mName << '|'
        << obj.mMetaDataKnown << '|'
        << obj.mNumberOfFrames << '|'
        << obj.mHasVideo << '|'
        << obj.mHasAudio << '|'
        << obj.mFileOpened << '|'
        << obj.mFileOpenedOk << '|'
        << obj.mReadingPackets << '|'
        << obj.mEOF << '|'
        << obj.mStreamIndex << '|'
        << obj.mMaxBufferSize << '|'
        << obj.mTwoInARow << '|'
        << obj.mFileContext;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void File::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        boost::serialization::void_cast_register<File, IFile>(
            static_cast<File*>(0),
            static_cast<IFile*>(0)
            );
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Node);
        ar & boost::serialization::make_nvp("mPath", mPath);
        if (version == 1)
        {
            time_t mLastModified;
            ar & BOOST_SERIALIZATION_NVP(mLastModified);
        }
        ar & BOOST_SERIALIZATION_NVP(mMaxBufferSize);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void File::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void File::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::File)
