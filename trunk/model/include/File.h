#ifndef MODEL_FILE_H
#define MODEL_FILE_H

#include <list>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "Node.h"
#include "IControl.h"
#include "IPath.h"
#include "UtilFrameRate.h"
#include "FilePacket.h"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
enum AVMediaType;

namespace model {

class File;
typedef boost::shared_ptr<File> FilePtr;
typedef std::list<FilePtr> Files;

class File
    :   public IControl
    ,   public Node
    ,   public IPath
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    File();
    File(wxFileName path, int buffersize = 1);
    virtual File* clone() override;
    virtual ~File();

    void abort();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IPATH
    //////////////////////////////////////////////////////////////////////////

    wxFileName getPath() const override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Returns the full path if the parent node is not an AutoFolder. Returns 
    /// the filename only if the parent node is an AutoFolder.
    wxString getName() const override;

    wxDateTime getLastModified() const;

    bool isSupportedFileType(); ///< \return true if this file is of a known format
    bool canBeOpened();         ///< \return true if this file can be opened properly

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

    /// If all packets have been retrieved return true. In that case,
    /// first a moveTo() is required to reset the EOF flag.
    bool getEOF() const;

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
    wxFileName mPath;
    mutable boost::optional<wxDateTime> mLastModified;
    bool mHasVideo;
    bool mHasAudio;
    bool mCanBeOpened;

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

#endif // MODEL_FILE_H
