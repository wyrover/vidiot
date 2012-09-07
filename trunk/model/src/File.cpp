#include "File.h"

#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "AudioFile.h"
#include "AutoFolder.h"
#include "Convert.h"
#include "Dialog.h"
#include "FilePacket.h"

#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoFile.h"

namespace model {

boost::mutex File::sMutexAvcodec;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

File::File()
:	IFile()
,   Node()
,   mPath()
,   mName()
,	mFileContext(0)
,   mReadingPackets(false)
,   mEOF(false)
,   mPackets(1)
,   mMaxBufferSize(0)
,   mStreamIndex(-1)
,   mBufferPacketsThreadPtr()
,   mFileOpen(false)
,   mNumberOfFrames(0)
,   mTwoInARow(0)
,   mLastModified(boost::none)
,   mHasVideo(false)
,   mHasAudio(false)
,   mCanBeOpened(false)
{
    VAR_DEBUG(this);
}

File::File(wxFileName path, int buffersize)
:	IFile()
,   Node()
,   mPath(path)
,   mName()
,	mFileContext(0)
,   mReadingPackets(false)
,   mEOF(false)
,   mPackets(1)
,   mMaxBufferSize(buffersize)
,   mStreamIndex(-1)
,   mBufferPacketsThreadPtr()
,   mFileOpen(false)
,   mNumberOfFrames(0)
,   mTwoInARow(0)
,   mLastModified(boost::none)
,   mHasVideo(false)
,   mHasAudio(false)
,   mCanBeOpened(false)
{
    VAR_DEBUG(this);
    if (isSupportedFileType())
    {
        // These two lines are required to correctly read the length
        // (and/or any other meta data) from the file.
        // This can only be done for supported formats, since avcodec
        // can only read the lengths from those.
        //
        // Note that the opening of a file sets mCanBeOpened
        // to the correct value.
        openFile();
        closeFile();
    }
}

File::File(const File& other)
:	IFile()
,   Node()
,   mPath(other.mPath)
,   mName(other.mName)
,	mFileContext(0)
,   mReadingPackets(false)
,   mEOF(false)
,   mPackets(1)
,   mMaxBufferSize(other.mMaxBufferSize)
,   mStreamIndex(-1)
,   mBufferPacketsThreadPtr()
,   mFileOpen(false)
,   mNumberOfFrames(other.mNumberOfFrames)
,   mTwoInARow(0)
,   mLastModified(other.mLastModified)
,   mHasVideo(other.mHasVideo)
,   mHasAudio(other.mHasAudio)
,   mCanBeOpened(other.mCanBeOpened)
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
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts File::getLength() const
{
    return mNumberOfFrames;
}

void File::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
    openFile(); // Needed for avcodec calls below

    stopReadingPackets();

    int64_t timestamp = model::Convert::ptsToMicroseconds(position);
    ASSERT_LESS_THAN_EQUALS(timestamp,mFileContext->duration)(timestamp)(mFileContext);
    VAR_DEBUG(timestamp)(mFileContext->duration);
    int result = av_seek_frame(mFileContext, -1, timestamp, AVSEEK_FLAG_ANY);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

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

wxDateTime File::getLastModified() const
{
    if (!mLastModified)
    {
        mLastModified = boost::optional<wxDateTime>(mPath.GetModificationTime());
    }
    return *mLastModified;
}

wxString File::getName() const
{
    if (hasParent() && getParent()->isA<AutoFolder>())
    {
        return util::path::toName(mPath);
    }
    return mPath.GetLongPath();
};

bool File::isSupportedFileType()
{
    if (mPath.GetExt().IsSameAs("avi"))
    {
        return true;
    }
    return false;
}

bool File::canBeOpened()
{
    return mCanBeOpened;
}

bool File::hasVideo()
{
    openFile();
    return mHasVideo;
}

bool File::hasAudio()
{
    openFile();
    return mHasAudio;
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
    if (mReadingPackets) return;

    VAR_DEBUG(this);

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    mReadingPackets = true;
    mBufferPacketsThreadPtr.reset(new boost::thread(boost::bind(&File::bufferPacketsThread,this)));

    VAR_DEBUG(this);
}

void File::stopReadingPackets()
{
    VAR_DEBUG(this)(mReadingPackets)(mEOF);
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
    if (mFileOpen) return;

    VAR_DEBUG(this);

    int result = 0;

    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        result = avformat_open_input(&mFileContext, mPath.GetLongPath(), 0, 0);
    }
    if (result != 0)
    {
        // Some error occured when opening the file.
        VAR_WARNING(mPath)(Avcodec::getErrorMessage(result));
        return;
    }

    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        result = av_find_stream_info(mFileContext);
    }
    if (result < 0)
    {
        // Some error occured when opening the file.
        VAR_WARNING(Avcodec::getErrorMessage(result));
        return;
    }

    mNumberOfFrames = -1;
    for (unsigned int i=0; i < mFileContext->nb_streams; ++i)
    {
        AVStream* stream = mFileContext->streams[i];
        VAR_DEBUG(stream);

        if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            mHasVideo = true;

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
            mHasAudio = true;

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
    mCanBeOpened = true;
}

void File::closeFile()
{
    VAR_DEBUG(this);
    if (!mFileOpen) return;

    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        avformat_close_input(&mFileContext);
        ASSERT_ZERO(mFileContext);
        mFileOpen = false;
    }
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
    os << &obj << '|' << obj.mPath << '|' << obj.mFileOpen << '|' << obj.mReadingPackets << '|' << obj.mEOF << '|' << obj.mMaxBufferSize << '|' << obj.mNumberOfFrames << '|' << obj.mTwoInARow << '|' << obj.mLastModified;
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
    ar & mMaxBufferSize;
}

template void File::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void File::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace