#ifndef MODEL_FILE_H
#define MODEL_FILE_H

#include <wx/filename.h>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include "IControl.h"
#include "FilePacket.h"
#include "AProjectViewNode.h"
#include "UtilFrameRate.h"

// FORWARD DECLARATIONS
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
enum AVMediaType;

namespace model {

class File;
typedef boost::shared_ptr<File> FilePtr;

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
    virtual File* clone();
    virtual ~File();

    void abort();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength();
    virtual void moveTo(pts position);
    virtual wxString getDescription() const;
    virtual void clean();

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

    wxFileName getFileName() const;
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
    /// \see make_cloned
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

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::mutex sMutexStop; ///< This mutex is needed to ensure that any pending getNextPacket() - which is executed in an external thread - is finished when stopping.
    wxString mName;
    AVFormatContext* mFileContext;
    int mStreamIndex;
    bool mFileOpen;
    bool mReadingPackets;
    bool mEOF;
    int mMaxBufferSize;
    FifoPacket mPackets; ///< Holds retrieved packets until extracted with getNextPacket()
    pts mNumberOfFrames;
    int mTwoInARow;
    boost::scoped_ptr<boost::thread> mBufferPacketsThreadPtr;
    boost::filesystem::path mPath;
    mutable boost::optional<wxString> mLastModified;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void openFile();
    void closeFile();

    //////////////////////////////////////////////////////////////////////////
    // THREADS
    //////////////////////////////////////////////////////////////////////////

    void bufferPacketsThread();

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
