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

#pragma once

#include "FilePacket.h"
#include "IFile.h"
#include "IPath.h"
#include "Node.h"
#include "UtilEnum.h"
#include "UtilFrameRate.h"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
enum AVMediaType;

namespace model {

DECLAREENUM(FileType, \
    FileType_Video, \
    FileType_Audio, \
    FileType_Image, \
    FileType_Title);

class File
    :   public IFile
    ,   public Node
    ,   public IPath
{
public:

    static wxString sSupportedVideoExtensions;
    static wxString sSupportedAudioExtensions;
    static wxString sSupportedImageExtensions;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    File();

    File(const wxFileName& path, int buffersize = 1);

    virtual File* clone() const;

    virtual void onCloned() override;

    virtual ~File();

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    NodePtrs findPath(const wxString& path) override;
    bool mustBeWatched(const wxString& path) override;
    void check(bool immediately = false) override;

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

    /// Initialize the meta data of the file object
    void readMetaData();

    /// \return true if this file can be opened properly
    virtual bool canBeOpened();

    bool hasVideo();
    bool hasAudio();

    /// Determine file type based on the file name. No inspection of the file may
    /// be done, since this method is also used for cases where file opening is
    /// not allowed (in the project view) for performance reasons.
    /// \return type of this file
    FileType getType() const;

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
    virtual bool useStream(const AVMediaType& type) const;

    /// \return the stream used by this file
    /// \return 0 if file could not be opened (or is empty), or if no matching stream could be found.
    /// File is opened if it was not yet opened
    AVStream* getStream();

    int64_t getStreamStartPosition();

    //////////////////////////////////////////////////////////////////////////
    // PACKETS INTERFACE TO SUBCLASSES
    //////////////////////////////////////////////////////////////////////////

    /// File is opened if it was not yet opened
    void startReadingPackets();

    void stopReadingPackets();

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

    // Attributes
    wxFileName mPath;
    wxString mName;
    mutable boost::optional<pts> mNumberOfFrames;
    bool mHasVideo = false;
    bool mHasAudio = false;
    int64_t mMaximumStartPts = 0; ///< Holds the maximum of the start positions of any stream. Any packet that is before this, is discarded, to ensure audio-video sync.

    // Status of opening
    bool mMetaDataKnown = false;    ///< True if the meta data (file path exists, canBeOpened) has been retrieved.
    bool mFileOpened = false;       ///< True if the file open has been done. Note: The file open may have failed.
    bool mFileOpenedOk = false;     ///< True if the file has been opened, with success.
    bool mReadingPackets = false;
    bool mEOF = false;

    // AVCodec access
    AVFormatContext* mFileContext = nullptr;
    int mStreamIndex = STREAMINDEX_UNDEFINED;

    // Buffering
    int mMaxBufferSize = 0;
    FifoPacket mPackets; ///< Holds retrieved packets until extracted with getNextPacket()
    int mTwoInARow = 0;
    std::unique_ptr<boost::thread> mBufferPacketsThreadPtr = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts getNumberOfFrames() const;

    /// If the file is not yet opened, it is opened. Otherwise, this call does nothing.
    ///
    /// If the file is not yet opened, and the meta data is not known, that meta data
    /// (modification time, whether the file can be opened, clip length) is retrieved.
    /// This can only be done for supported formats, since avcodec
    /// can only read the lengths from those.
    ///
    /// Also sets mFileOpenFailed to the correct value.
    void openFile();

    void closeFile();

    //////////////////////////////////////////////////////////////////////////
    // THREADS
    //////////////////////////////////////////////////////////////////////////

    void bufferPacketsThread();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const File& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::File, 2)
BOOST_CLASS_EXPORT_KEY(model::File)
