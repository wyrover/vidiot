#ifndef AUTO_FOLDER_H
#define AUTO_FOLDER_H

#include "Folder.h"
#include "IPath.h"

namespace worker {
    class WorkDoneEvent;
}

namespace model {

struct IndexAutoFolderWork;

class AutoFolder
    :   public Folder
    ,   public IPath
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Recovery constructor.
    AutoFolder();

    /// \param path full path to the folder.
    AutoFolder(wxFileName path);

    virtual ~AutoFolder();

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    NodePtrs findPath(wxString path) override;
    bool mustBeWatched(wxString path) override;

    //////////////////////////////////////////////////////////////////////////
    // IPATH
    //////////////////////////////////////////////////////////////////////////

    wxFileName getPath() const override;

    //////////////////////////////////////////////////////////////////////////
    // STRUCTURE
    //////////////////////////////////////////////////////////////////////////

    /// Update the autofolder children. The folder is synced with the filesystem.
    void update();

    /// Called when the updating is done
    void onWorkDone(worker::WorkDoneEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    /// Returns the full path if the parent node is not an AutoFolder. Returns
    /// the relatve path only if the parent node is an AutoFolder.
    wxString getName() const override;

    time_t getLastModified() const;

    wxString getSequenceName() const override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxFileName mPath;
    time_t mLastModified;
    boost::shared_ptr<IndexAutoFolderWork> mCurrentUpdate;
    bool mUpdateAgain; ///< True if an update event was received while an update was already scheduled. To avoid a new file being found twice, no two updates are scheduled simultaneously.

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::AutoFolder, 1)

#endif // AUTO_FOLDER_H