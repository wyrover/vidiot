#ifndef AUTO_FOLDER_H
#define AUTO_FOLDER_H

#include <wx/filename.h>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include "Folder.h"

namespace model {

class AutoFolder;
typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;

class AutoFolder
    :   public Folder
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Recovery constructor.
    AutoFolder();                       

    /// \param path full path to the folder.
    AutoFolder(boost::filesystem::path path);

    virtual ~AutoFolder();

    //////////////////////////////////////////////////////////////////////////
    // STRUCTURE
    //////////////////////////////////////////////////////////////////////////

    /// \return list of supported files in the given directory. Folders are returned also.
    /// \param directory absolute path which is searched for files.
    /// Note that only supported file types - as indicated by File::isSupported() - are returned.
    static model::ProjectViewPtrs getSupportedFiles( boost::filesystem::path directory );

    /// Update the autofolder children. The folder is synced with the filesystem. 
    void update();

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    wxFileName getFileName() const;
    boost::filesystem::path getPath() const;
    wxString getName() const;
    boost::optional<wxString> getLastModified() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::filesystem::path mPath;
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
