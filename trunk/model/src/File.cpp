#include "File.h"

// Include at top, to exclude the intmax macros and use the boost versions
#undef INTMAX_C
#undef UINTMAX_C
extern "C" {
#include <avformat.h>
};

#include <math.h>
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/optional.hpp>
#include "FilePacket.h"
#include "Convert.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoFile.h"
#include "AudioFile.h"

namespace model {

boost::mutex File::sMutexAvcodec;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

File::File()
:	IControl()
,   AProjectViewNode()
,   mPath()
,   mName()
,	mFileContext(0)
,   mReadingPackets(false)
,   mEOF(false)
,   mPackets(1)
,   mMaxBufferSize(0)
,   mStreamIndex(-1)
,   mBufferPacketsThreadPtr(0)
,   mFileOpen(false)
,   mNumberOfFrames(0)
,   mTwoInARow(0)
,   mLastModified(boost::none)
{
    VAR_DEBUG(this);
}

File::File(boost::filesystem::path path, int buffersize)
:	IControl()
,   AProjectViewNode()
,   mPath(path)
,   mName()
,	mFileContext(0)
,   mReadingPackets(false)
,   mEOF(false)
,   mPackets(1)
,   mMaxBufferSize(buffersize)
,   mStreamIndex(-1)
,   mBufferPacketsThreadPtr(0)
,   mFileOpen(false)
,   mNumberOfFrames(0)
,   mTwoInARow(0)
,   mLastModified(boost::none)
{
    VAR_DEBUG(this);
}

File::File(const File& other)
:	IControl()
,   AProjectViewNode()
,   mPath(other.mPath)
,   mName(other.mName)
,	mFileContext(0)
,   mReadingPackets(false)
,   mEOF(false)
,   mPackets(1)
,   mMaxBufferSize(other.mMaxBufferSize)
,   mStreamIndex(-1)
,   mBufferPacketsThreadPtr(0)
,   mFileOpen(false)
,   mNumberOfFrames(0) // For a copy we read the number of packets again from the file in getLength()
,   mTwoInARow(0)
,   mLastModified(other.mLastModified)
{
    VAR_DEBUG(this);
}

File* File::clone()
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
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts File::getLength()
{
    openFile();
    return mNumberOfFrames;
}

void File::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
    openFile(); // Needed for avcodec calls below

    stopReadingPackets();

    int result = av_seek_frame(mFileContext, -1, model::Convert::ptsToMicroseconds(position), AVSEEK_FLAG_ANY);
    ASSERT(result >= 0)(result);

    ASSERT(mPackets.getSize() == 0)(mPackets.getSize());
    mPackets.resize(1); // Ensures that only one packet is buffered (used for thumbnail generation).
    mTwoInARow = 0;

    mEOF = false;

    //startReadingPackets();
    VAR_DEBUG(this);
}

wxString File::getDescription() const
{
    return getName();
}


void File::clean()
{
    VAR_DEBUG(this);
    stopReadingPackets();
    closeFile();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxFileName File::getFileName() const
{
    return wxFileName(mPath.parent_path().string(),mPath.filename().string());
}

boost::filesystem::path File::getPath() const
{
    return mPath;
}

wxString File::getLastModified() const
{
    if (!mLastModified)
    {
        boost::posix_time::ptime lwt = boost::posix_time::from_time_t(boost::filesystem::last_write_time(mPath));
        mLastModified = boost::optional<wxString>(boost::posix_time::to_simple_string(lwt));
    }
    return *mLastModified;
}

wxString File::getName() const
{
    return mPath.filename().string();
};

bool File::isSupported()
{
    if (mPath.extension().string().compare(".avi") == 0)
    {
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// PACKETS INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

void File::startReadingPackets()
{
    openFile();
    if (mReadingPackets) return;

    VAR_DEBUG(this);

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    mReadingPackets = true;
    mBufferPacketsThreadPtr.reset(new boost::thread(boost::bind(&File::bufferPacketsThread,this)));

    VAR_DEBUG(this);
}

void File::stopReadingPackets()
{
    VAR_DEBUG(this);
    if (!mReadingPackets && !mEOF) return; // !mEOF is needed since we still want the buffers to be cleared, in the case that bufferPacketsThread has already delivered the last packet

    boost::mutex::scoped_lock lock(sMutexAvcodec);

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

    VAR_DEBUG(this);
}

void File::flush()
{
}

AVCodecContext* File::getCodec()
{
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

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void File::openFile()
{
    if (mFileOpen) return;

    VAR_DEBUG(this);

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    int result = av_open_input_file(&mFileContext, mPath.string().c_str(), NULL, 0, NULL);
    ASSERT(result == 0)(result);

    result = av_find_stream_info(mFileContext);
    ASSERT(result >= 0)(result);

    mNumberOfFrames = -1;
    for (unsigned int i=0; i < mFileContext->nb_streams; ++i)
    {
        AVStream* stream = mFileContext->streams[i];
        VAR_DEBUG(stream);

        if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            // If there is video in the file, then the number of video frames is used for the duration.
            FrameRate videoFrameRate = FrameRate(stream->codec->time_base.num, stream->codec->time_base.den);
            mNumberOfFrames = Convert::toProjectFrameRate(stream->duration, videoFrameRate);
            VAR_DEBUG(mNumberOfFrames);

            if (isA<VideoFile>())
            {
                mStreamIndex = i;
            }
        }
        else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            if (isA<AudioFile>())
            {
                mStreamIndex = i;
            }
        }
    }
    if (mNumberOfFrames == -1)
    {
        // For files without video, determine the number of 'virtual video frames'.
        mNumberOfFrames = Convert::microsecondsToPts(mFileContext->streams[mStreamIndex]->duration);
    }
    VAR_DEBUG(mFileContext)(mStreamIndex)(mNumberOfFrames);
    mFileOpen = true;
}

void File::closeFile()
{
    VAR_DEBUG(this);
    if (!mFileOpen) return;

    av_close_input_file(mFileContext);
    mFileOpen = false;
}

//////////////////////////////////////////////////////////////////////////
// THREADS
//////////////////////////////////////////////////////////////////////////

void File::bufferPacketsThread()
{
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
        ASSERT(packet->size > 0);

        int retval = av_dup_packet(packet);
        ASSERT(retval >= 0)(retval);

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
    os << &obj << '|' << obj.mPath << '|' << obj.mFileOpen << '|' << obj.mReadingPackets << '|' << obj.mEOF << '|' << obj.mMaxBufferSize << '|' << obj.mNumberOfFrames << '|' << obj.mTwoInARow << '|' << obj.mLastModified;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void File::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & boost::serialization::base_object<AProjectViewNode>(*this);
    ar & mPath;
    ar & mLastModified;
    ar & mMaxBufferSize;
}

template void File::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void File::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace

