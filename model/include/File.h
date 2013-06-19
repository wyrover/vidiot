#ifndef MODEL_FILE_H
#define MODEL_FILE_H

#include "FilePacket.h"
#include "IFile.h"
#include "IPath.h"
#include "Node.h"
#include "UtilFrameRate.h"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
enum AVMediaType;

namespace model {

class File
    :   public IFile
    ,   public Node
    ,   public IPath
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    File();
    File(wxFileName path, int buffersize = 1);
    virtual File* clone() const override;
    virtual ~File();

    void abort();

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    NodePtrs findPath(wxString path) override;
    bool mustBeWatched(wxString path) override;

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
    void check() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Returns the full path if the parent node is not an AutoFolder. Returns
    /// the filename only if the parent node is an AutoFolder.
    wxString getName() const override;

    time_t getLastModified() const;

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
    // STREAMS INTERFACE TO SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    static const int STREAMINDEX_UNDEFINED;
    static const int LENGTH_UNDEFINED;

    /// \return true if the given stream is applicable to this object.
    /// Per default returns false in the base implementation
    virtual bool useStream(AVMediaType type) const;

    /// \return the stream used by this file
    /// \return 0 if file could not be opened (or is empty), or if no matching stream could be found.
    /// File is opened if it was not yet opened
    AVStream* getStream();

    //////////////////////////////////////////////////////////////////////////
    // PACKETS INTERFACE TO SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    /// File is opened if it was not yet opened
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
    time_t mLastModified;
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