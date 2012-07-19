#ifndef AUTO_FOLDER_H
#define AUTO_FOLDER_H

#include "Folder.h"
#include "IPath.h"

namespace model {

class AutoFolder;
typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;

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
    // IPATH
    //////////////////////////////////////////////////////////////////////////

    wxFileName getPath() const override;

    //////////////////////////////////////////////////////////////////////////
    // STRUCTURE
    //////////////////////////////////////////////////////////////////////////

    /// \return list of supported files in the given directory. Folders are returned also.
    /// \param path absolute path which is searched for files.
    /// Note that only supported file types - as indicated by File::isSupportedFileType() - are returned.
    /// Furthermore, not that files are opened with avcodec. If that returns an error, the file
    /// is not added to the returned list - see File::canBeOpened().
    static IPaths getSupportedFiles( wxFileName path );

    /// Update the autofolder children. The folder is synced with the filesystem.
    void update();

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    /// Returns the full path if the parent node is not an AutoFolder. Returns
    /// the relatve path only if the parent node is an AutoFolder.
    wxString getName() const override;

    boost::optional<wxString> getLastModified() const;

    wxString getSequenceName() const override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxFileName mPath;
    boost::optional<wxString> mLastModified;

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