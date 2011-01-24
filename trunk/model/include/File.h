#ifndef MODEL_FILE_H
#define MODEL_FILE_H

#include <ctime>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include "IControl.h"
#include "FilePacket.h"
#include "AProjectViewNode.h"
#include "FrameRate.h"

// Forward declarations of ffmpeg types
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
enum AVMediaType;

namespace model {

class File
    :   public IControl
    ,   public AProjectViewNode
    //:   public AProjectViewNode
    //,   public IControl
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    File();
    File(boost::filesystem::path path, int buffersize = 1);
    virtual File* clone();
    virtual ~File();

    void abort();

	//////////////////////////////////////////////////////////////////////////
	// ICONTROL
	//////////////////////////////////////////////////////////////////////////

    virtual pts getNumberOfFrames();
    virtual void moveTo(pts position);
    virtual wxString getDescription() const;

    //////////////////////////////////////////////////////////////////////////
    // FOR DETERMINING THE TYPE OF FILE
    //////////////////////////////////////////////////////////////////////////

    template <typename Derived>
    bool isA()
    {
        return (typeid(Derived) == typeid(*this));
    }

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxString getName() const;
    boost::filesystem::path getPath() const;
    wxString getLastModified() const;
    bool isSupported();

    bool hasVideo();
    bool hasAudio();

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// @see make_cloned
    File(const File& other);

    //////////////////////////////////////////////////////////////////////////
    // AVCODEC
    //////////////////////////////////////////////////////////////////////////

    /// This mutex is needed to ensure that never multiple avcodec open/close
    /// calls are executed in parallel.
    static boost::mutex sMutexAvcodec;

    //////////////////////////////////////////////////////////////////////////
    // PACKETS INTERFACE TO SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    void startReadingPackets();
    void stopReadingPackets();

    /// This method is called when the reading/decoding process must be restarted.
    /// Default behavior is to do nothing. Derived classes can reimplement this 
    /// to flush any pending (avcodec) buffers when the decoding is stopped/restarted
    /// for some reason (for instance, in case of moveTo()).
    virtual void flush();

    /// Return codec used for the current stream
    AVCodecContext* getCodec();

    /// Retrieve next buffered packet. Blocks if there's no
    /// such packet. Returns '0' packet in case there are no more
    /// packets in the file or if buffering of packets is stopped.
    PacketPtr getNextPacket();


private:

    /// This mutex is needed to ensure that any pending getNextPacket() - which is
    /// executed in an external thread - is finished when stopping.
    boost::mutex sMutexStop;

    wxString mName;

    //////////////////////////////////////////////////////////////////////////
    // AVCODEC
    //////////////////////////////////////////////////////////////////////////

    AVFormatContext* mFileContext;
    int mStreamIndex;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    void openFile();
    void closeFile();

    bool mFileOpen;

    bool mReadingPackets;
    bool mEOF;

    int mMaxBufferSize;

    
    FifoPacket mPackets; ///< Holds retrieved packets until extracted with getNextPacket()

    pts mNumberOfFrames;
    int mTwoInARow;

    //////////////////////////////////////////////////////////////////////////
    // DISK IO
    //////////////////////////////////////////////////////////////////////////

    boost::scoped_ptr<boost::thread> mBufferPacketsThreadPtr;
    void bufferPacketsThread();

    boost::filesystem::path mPath;
    mutable boost::optional<wxString> mLastModified;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const File& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::File, 1)
BOOST_CLASS_EXPORT(model::File)

#endif // MODEL_FILE_H
