#include "File.h"

#include "AutoFolder.h"
#include "Constants.h"
#include "Convert.h"
#include "Dialog.h"
#include "FilePacket.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilThread.h"

namespace model {

// static
const int File::STREAMINDEX_UNDEFINED = -1;
const int File::LENGTH_UNDEFINED = -1;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

File::File()
    :	IFile()
    ,   Node()
    // Attributes
    ,   mPath()
    ,   mName()
    ,   mNumberOfFrames(LENGTH_UNDEFINED)
    ,   mLastModified(0)
    ,   mHasVideo(false)
    ,   mHasAudio(false)
    // Status of opening
    ,   mFileOpened(false)
    ,   mFileOpenFailed(false)
    ,   mReadingPackets(false)
    ,   mEOF(false)
    // AVCodec access
    ,	mFileContext(0)
    ,   mStreamIndex(STREAMINDEX_UNDEFINED)
    // Buffering
    ,   mMaxBufferSize(0)
    ,   mPackets(1)
    ,   mBufferPacketsThreadPtr()
    ,   mTwoInARow(0)
{
    VAR_DEBUG(this);
}

File::File(wxFileName path, int buffersize)
    :	IFile()
    ,   Node()
    // Attributes
    ,   mPath(path)
    ,   mName()
    ,   mNumberOfFrames(LENGTH_UNDEFINED)
    ,   mLastModified(0)
    ,   mHasVideo(false)
    ,   mHasAudio(false)
    // Status of opening
    ,   mFileOpened(false)
    ,   mFileOpenFailed(false)
    ,   mReadingPackets(false)
    ,   mEOF(false)
    // AVCodec access
    ,	mFileContext(0)
    ,   mStreamIndex(STREAMINDEX_UNDEFINED)
    // Buffering
    ,   mMaxBufferSize(buffersize)
    ,   mPackets(1)
    ,   mBufferPacketsThreadPtr()
    ,   mTwoInARow(0)
{
    VAR_DEBUG(this);

    if (mPath.Exists())
    {
        wxDateTime dt = mPath.GetModificationTime();
        if (dt.IsValid())
        {
            mLastModified = dt.GetTicks();

            // These two lines are required to correctly read the length
            // (and/or any other meta data) from the file.
            // This can only be done for supported formats, since avcodec
            // can only read the lengths from those.
            //
            // Note that the opening of a file sets mFileOpenFailed to the correct value.
            openFile();
            closeFile();
        }
    }

}

File::File(const File& other)
    :	IFile()
    ,   Node()
    // Attributes
    ,   mPath(other.mPath)
    ,   mName(other.mName)
    ,   mNumberOfFrames(other.mNumberOfFrames)
    ,   mLastModified(other.mLastModified)
    ,   mHasVideo(other.mHasVideo)
    ,   mHasAudio(other.mHasAudio)
    // Status of opening
    ,   mFileOpened(false)
    ,   mFileOpenFailed(false)
    ,   mReadingPackets(false)
    ,   mEOF(false)
    // AVCodec access
    ,	mFileContext(0)
    ,   mStreamIndex(STREAMINDEX_UNDEFINED)
    // Buffering
    ,   mMaxBufferSize(other.mMaxBufferSize)
    ,   mPackets(1)
    ,   mBufferPacketsThreadPtr()
    ,   mTwoInARow(0)
{
    VAR_DEBUG(this);
}

File* File::clone() const
{
    return new File(static_cast<const File&>(*this));
}

File::~File()
{
    VAR_DEBUG(this);
    stopReadingPackets();
    closeFile();
}

void File::abort()
{
    VAR_DEBUG(this);
    stopReadingPackets();
    closeFile();
}

//////////////////////////////////////////////////////////////////////////
// INODE
//////////////////////////////////////////////////////////////////////////

NodePtrs File::findPath(wxString path)
{
    NodePtrs result;
    if (util::path::equals(mPath,path))
    {
        result.push_back(shared_from_this());
    }
    return result;
}

bool File::mustBeWatched(wxString path)
{
    if (util::path::isParentOf(path,mPath))
    {
        // Yes, is parent folder of this file
        return true;
    }
    return false;
}

void File::check()
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
            gui::Dialog::get().getConfirmation(_("File removed"), _("The file ") + util::path::toName(mPath) + _(" has been removed from disk. File is removed from project also."));
            parent->removeChild(shared_from_this());
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
    ASSERT_MORE_THAN_ZERO(mNumberOfFrames);

    return mNumberOfFrames;
}

void File::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
    openFile(); // Needed for avcodec calls below

    if (fileOpenFailed()) { return; } // File probably closed

    stopReadingPackets();

    int64_t timestamp = model::Convert::ptsToMicroseconds(position);
    ASSERT_LESS_THAN_EQUALS(timestamp,mFileContext->duration)(timestamp)(mFileContext)(position);
    VAR_DEBUG(timestamp)(mFileContext->duration);
    int result = av_seek_frame(mFileContext, -1, timestamp, AVSEEK_FLAG_ANY);
    ASSERT_MORE_THAN_EQUALS_ZERO(result)(avcodecErrorString(result)); // todo log *this upon errors for easier analysis

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

time_t File::getLastModified() const
{
    return mLastModified;
}

wxString File::getName() const
{
    if (hasParent() && getParent()->isA<AutoFolder>())
    {
        return util::path::toName(mPath);
    }
    return mPath.GetLongPath();
};

bool File::canBeOpened()
{
    return !mFileOpenFailed;
}

bool File::hasVideo()
{
    return mHasVideo;
}

bool File::hasAudio()
{
    return mHasAudio;
}

//////////////////////////////////////////////////////////////////////////
// STREAMS INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

bool File::useStream(AVMediaType type) const
{
    return false;
}

AVStream* File::getStream()
{
    openFile();
    if (!fileOpenFailed() && mFileContext && mStreamIndex != STREAMINDEX_UNDEFINED)
    {
        return mFileContext->streams[mStreamIndex];
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// PACKETS INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

bool File::fileOpenFailed() const
{
    return mFileOpenFailed;
}

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
    if (fileOpenFailed()) { return; } // File probably closed

    if (mReadingPackets) return;

    VAR_DEBUG(this);

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    mReadingPackets = true;
    mBufferPacketsThreadPtr.reset(new boost::thread(boost::bind(&File::bufferPacketsThread,this)));

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
    flush();

    // From this point onwards, startReadingPackets should initalize reading
    // again.
    mEOF = false;

    VAR_DEBUG(this);
}

void File::flush()
{
}

AVCodecContext* File::getCodec()
{
    openFile();
    if (fileOpenFailed()) { return 0; } // File probably closed

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
    VAR_DETAIL(packet)(mPackets.getSize());
    return packet;
}

bool File::getEOF() const
{
    return mEOF;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void File::openFile()
{
    if (mFileOpened) return;
    mFileOpened = true;
    VAR_DEBUG(this);

    mFileOpenFailed = true; // Is reset after the open succeeds

    int result = 0;
    wxString path = mPath.GetLongPath();

    {
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        result = avformat_open_input(&mFileContext, path, 0, 0);
    }
    if (result != 0)
    {
        // Some error occured when opening the file.
        VAR_DEBUG(path)(result)(avcodecErrorString(result));
        return;
    }
    {
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        result = avformat_find_stream_info(mFileContext,0);
        if (result < 0) // Some error occured when reading stream info. Close the file again.
        {
            VAR_DEBUG(path)(result)(avcodecErrorString(result));
            avformat_close_input(&mFileContext); // Requires the lock also
            ASSERT_ZERO(mFileContext);
            return;
        }
    }

    auto getStreamLength = [this](AVStream* stream) -> pts
    {
        return Convert::rationaltimeToPts(boost::rational<int>(Constants::sSecond,1) * boost::rational<int>(stream->duration,1) * boost::rational<int>(stream->time_base.num,stream->time_base.den));
    };

    auto isAudioSupported = [this,path](AVStream* stream) -> bool
    {
        if (stream->codec->codec_type != AVMEDIA_TYPE_AUDIO)
        {
            return false;
        }
        VAR_DEBUG(stream->codec->sample_fmt)(stream->codec->channels)(stream->codec->sample_rate);
        if ((stream->codec->channels < 0) ||  (stream->codec->channels > 2))
        {
            LOG_WARNING << "Unsupported audio file '" << path << "'. Number of channels is " << stream->codec->channels << ".";
            return false;
        }
        switch (stream->codec->sample_fmt)
        {
            // Supported/tested sample formats
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_DBL:
            break;
            // Unsupported/untested sample formats
        case AV_SAMPLE_FMT_NONE:
        case AV_SAMPLE_FMT_U8P:
        case AV_SAMPLE_FMT_S16P:
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_FLTP:
        case AV_SAMPLE_FMT_DBLP:
            LOG_WARNING << "Unsupported audio file '" << path << "'. Sample format is " << stream->codec->sample_fmt << ".";
            return false;
        }

        if (av_get_bytes_per_sample(stream->codec->sample_fmt) == 0)
        {
            LOG_WARNING << "Unsupported audio file '" << path << "'. Number of bytes per sample is unknown.";
            return false;
        }

        if (stream->codec->sample_rate < 4000 || stream->codec->sample_rate > 256000)
        {
            LOG_WARNING << "Unsupported audio file '" << path << "'. Sample rate (" << stream->codec->sample_rate << ") too big.";
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
        return true;
    };

    mNumberOfFrames = LENGTH_UNDEFINED;
    mStreamIndex = STREAMINDEX_UNDEFINED;
    for (unsigned int i=0; i < mFileContext->nb_streams; ++i)
    {
        AVStream* stream = mFileContext->streams[i];
        VAR_DEBUG(stream);

        if (isVideoSupported(stream))
        {
            mHasVideo = true;

            if ( (stream->duration != AV_NOPTS_VALUE) && (stream->duration != 0))
            {
                mNumberOfFrames = getStreamLength(stream);
            }
            else
            {
                if ( (stream->nb_frames != AV_NOPTS_VALUE) && (stream->nb_frames != 0))
                {
                    mNumberOfFrames = stream->nb_frames;
                }
                else
                {
                    mNumberOfFrames = 0;
                }
            }
            VAR_DEBUG(stream)(mNumberOfFrames);
        }
        else if (isAudioSupported(stream))
        {
            mHasAudio = true;

            if (mNumberOfFrames == LENGTH_UNDEFINED)
            {
                // For files without video, determine the number of 'virtual video frames'.
                mNumberOfFrames = getStreamLength(stream);
            }
        }
        else
        {
            continue; // To ensure that this stream is not used in case the video/audio contents is not supported
        }

        if ((mStreamIndex == STREAMINDEX_UNDEFINED) && useStream(stream->codec->codec_type))
        {
            mStreamIndex = i;
        }
    }

    if ((!mHasVideo && !mHasAudio) || (mNumberOfFrames <= 0)) // <= 0: Some files have streams, but with all lengths == 0 (once happened when indexing by mistake ffprobe.exe)
    {
        LOG_WARNING << "No correct stream found";
        boost::mutex::scoped_lock lock(Avcodec::sMutex);
        avformat_close_input(&mFileContext);
        ASSERT_ZERO(mFileContext);
        mNumberOfFrames = LENGTH_UNDEFINED;
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
    mFileOpenFailed = false;
}

void File::closeFile()
{
    VAR_DEBUG(this);
    if (!mFileOpened) { return; }
    if (fileOpenFailed()) { return; }

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

    AVPacket pkt1;
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
        ASSERT_MORE_THAN_ZERO(packet->size);

        int retval = av_dup_packet(packet);
        ASSERT_MORE_THAN_EQUALS_ZERO(retval);

        if(packet->stream_index == mStreamIndex)
        {
            PacketPtr p = boost::make_shared<Packet>(packet);
            mPackets.push(p);
            VAR_DETAIL(this)(p);
        }
        else
        {
            av_free_packet(packet);
        }
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

std::ostream& operator<<( std::ostream& os, const File& obj )
{
    os  << &obj << '|'
        << obj.mPath << '|'
        << obj.mName << '|'
        << obj.mNumberOfFrames << '|'
        << obj.mLastModified << '|'
        << obj.mHasVideo << '|'
        << obj.mHasAudio << '|'
        << obj.mFileOpened << '|'
        << obj.mFileOpenFailed << '|'
        << obj.mReadingPackets << '|'
        << obj.mEOF << '|'
        << obj.mStreamIndex << '|'
        << obj.mMaxBufferSize << '|'
        << obj.mTwoInARow;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void File::serialize(Archive & ar, const unsigned int version)
{
    boost::serialization::void_cast_register<File, IFile>(
        static_cast<File*>(0),
        static_cast<IFile*>(0)
        );
    ar & boost::serialization::base_object<Node>(*this);
    ar & mPath;
    ar & mLastModified;
    ar & mMaxBufferSize;
    if (Archive::is_loading::value)
    {
        // PERF: Cache each file once
        openFile();
        closeFile();
    }
}

template void File::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void File::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace