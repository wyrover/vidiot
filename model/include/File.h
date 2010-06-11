#ifndef MODEL_FILE_H
#define MODEL_FILE_H

#include <ctime>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include "IControl.h"
#include "FifoPacket.h"
#include "AProjectViewNode.h"

#pragma warning ( disable : 4005 ) // Redefinition of INTMAX_C/UINTMAX_C by boost and ffmpeg 

extern "C" {
#include <avformat.h>
};

namespace model {

class File 
    :   public IControl
    ,   public AProjectViewNode
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    File();
    File(boost::filesystem::path path, int buffersize = 1);
    virtual ~File();

	//////////////////////////////////////////////////////////////////////////
	// ICONTROL
	//////////////////////////////////////////////////////////////////////////

    virtual boost::int64_t getNumberOfFrames();
    virtual void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    wxString getName() const; 
    boost::filesystem::path getPath() const;
    wxString getLastModified() const;
    bool isSupported();

    bool hasVideo();
    bool hasAudio();

protected:

    //////////////////////////////////////////////////////////////////////////
    // AVCODEC
    //////////////////////////////////////////////////////////////////////////

    /**
    * This mutex is needed to ensure that never multiple avcodec open/close 
    * are executed in parallel.
    */
    static boost::mutex sMutexAvcodec;

    AVFormatContext* mFileContext;
    int mStreamIndex;
    AVStream* mStream;
    AVCodecContext* mCodecContext;
    CodecType mCodecType;

    //////////////////////////////////////////////////////////////////////////
    // PACKETS INTERFACE TO SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    void startReadingPackets();
    void stopReadingPackets();

    /**
    * Retrieve next buffered packet. Blocks if there's no
    * such packet. Returns '0' packet in case there are no more
    * packets in the file or if buffering of packets is stopped.
    */
    PacketPtr getNextPacket();


private:

    /**
    * This mutex is needed to ensure that any pending getNextPacket() - which is
    * executed in an external thread - is finished when stopping.
    */
    boost::mutex sMutexStop;

    wxString mName;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    void openFile();
    void closeFile();

    bool mFileOpen;

    bool mReadingPackets;

    int mMaxBufferSize;

    /** Holds retrieved packets until extracted with getNextPacket() */
    FifoPacket mPackets;

    int64_t mNumberOfFrames;
    int mTwoInARow;

    //////////////////////////////////////////////////////////////////////////
    // DISK IO
    //////////////////////////////////////////////////////////////////////////

    boost::scoped_ptr<boost::thread> mBufferPacketsThreadPtr;
    void bufferPacketsThread();

    boost::filesystem::path mPath;
    mutable boost::optional<wxString> mLastModified; 

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::File, 1)
BOOST_CLASS_EXPORT(model::File)

#endif // MODEL_FILE_H
