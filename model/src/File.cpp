#include "File.h"
#include <math.h>
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/optional.hpp>
#include "GuiTimeLineZoom.h"
#include "Project.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "AProjectViewNode.h"

namespace model {

boost::mutex File::sMutexAvcodec;

static double sMilliSecondsPerSecond = 1000.0; 
static double sMicroSecondsPerSecond = 1000.0 * sMilliSecondsPerSecond; 

static const int sBytesPerSample = 2;
static const double sVideoFrameRate = 25.0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

File::File()
:	IControl()
,   mPath()
,	mFileContext(0)
,   mReadingPackets(false)
,   mPackets(FifoPacket(1))
,   mMaxBufferSize(0)
,   mStreamIndex(-1)
,   mCodecContext(0)
,   mBufferPacketsThreadPtr(0)
,   mFileOpen(false)
,   mNumberOfFrames(0)
,   mTwoInARow(0)
,   mCodecType(CODEC_TYPE_UNKNOWN)
,   mLastModified(boost::none)
{ 
    VAR_DEBUG(this);
}

File::File(boost::filesystem::path path, int buffersize)
:	IControl()
,   mPath(path)
,	mFileContext(0)
,   mReadingPackets(false)
,   mPackets(FifoPacket(1))
,   mMaxBufferSize(buffersize)
,   mStreamIndex(-1)
,   mCodecContext(0)
,   mBufferPacketsThreadPtr(0)
,   mFileOpen(false)
,   mNumberOfFrames(0)
,   mTwoInARow(0)
,   mCodecType(CODEC_TYPE_UNKNOWN)
,   mLastModified(boost::none)
{ 
    VAR_DEBUG(this);
}

File::~File()
{
    VAR_DEBUG(this);
    stopReadingPackets();
    closeFile();
}

void File::openFile()
{
    if (mFileOpen) return;

    VAR_DEBUG(this);

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    int result = av_open_input_file(&mFileContext, mPath.string().c_str(), NULL, 0, NULL);
    ASSERT(result == 0)(result);

    result = av_find_stream_info(mFileContext);
    ASSERT(result >= 0)(result);

    /** /todo get all streams info, use that to make hasVideo and hasAudio for showing in project view. This class is about meta data of video/audio files.*/
    for (unsigned int i=0; i < mFileContext->nb_streams; ++i)
    {
        if (mFileContext->streams[i]->codec->codec_type == mCodecType)
        {
            mStream = mFileContext->streams[i];
            mStreamIndex = i;
            break;
        }
    }
    mNumberOfFrames = gui::timeline::GuiTimeLineZoom::microsecondsToPts(mFileContext->duration);

    VAR_DEBUG(mNumberOfFrames)(mFileContext->nb_streams)(mStreamIndex);

    mFileOpen = true;
}

void File::closeFile()
{
    if (mFileOpen)
    {
        VAR_DEBUG(this);

        av_close_input_file(mFileContext);
    }
    mFileOpen = false;
}

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

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    // Avoid new packets to be retrieved from the file in bufferPacketsThread.
    mReadingPackets = false;

    // Clear fifo to unblock any pending push in bufferPacketsThread.
    mPackets.flush();

    // Wait until thread ends.
    if (mBufferPacketsThreadPtr)
    {
        mBufferPacketsThreadPtr->join();
    }

    // When this lock is taken, it is certain that no 'pop' is 
    // blocking the return of getNextPacket().
    boost::mutex::scoped_lock stoplock(sMutexStop);

    // Clear buffers again, since one new (empty) packet may have 
    // been added when ending bufferPacketsThread. This is done 
    // AFTER taking the lock sMutexStop to avoid clearing the
    // '0' packet before getNextPacket() had a chance to return.
    mPackets.flush();

    // Finally, flush avcodec. The remaining avcodec buffers are no
    // longer necessary.
    if (mCodecContext)
    {
        avcodec_flush_buffers(mCodecContext);
    }

    VAR_DEBUG(this); 
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

boost::int64_t File::getNumberOfFrames()
{
    openFile();
    return mNumberOfFrames; 
}

void File::moveTo(boost::int64_t position)
{
    VAR_DEBUG(this)(position);
    openFile(); // Needed for avcodec calls below

    stopReadingPackets();

    int result = av_seek_frame(mFileContext, -1, gui::timeline::GuiTimeLineZoom::ptsToMicroseconds(position), AVSEEK_FLAG_ANY);
    ASSERT(result >= 0)(result);

    ASSERT(mPackets.getSize() == 0)(mPackets.getSize());
    mPackets.resize(1); // Ensures that only one packet is buffered (used for thumbnail generation).
    mTwoInARow = 0;
    startReadingPackets();
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ATTRIBUTES
//////////////////////////////////////////////////////////////////////////

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
    return mPath.leaf(); 
};

bool File::isSupported() 
{
    if (mPath.extension().compare(".avi") == 0) 
    {
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// PACKETS INTERFACE TO SUBCLASSES
//////////////////////////////////////////////////////////////////////////

PacketPtr File::getNextPacket()
{
    startReadingPackets(); // This is the normal trigger for starting to read

    // This lock is used to signal that an external thread is blocked on
    // getting a new packet.
    boost::mutex::scoped_lock stoplock(sMutexStop);

    if (++mTwoInARow == 2)
    {
        mPackets.resize(mMaxBufferSize);
    }
    PacketPtr packet = mPackets.pop();
    VAR_DETAIL(mCodecType)(packet)(mPackets.getSize());
    return packet;
}

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
    ar & mCodecType;
}
template void File::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void File::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
