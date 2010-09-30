#ifndef AUTO_FOLDER_H
#define AUTO_FOLDER_H

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include "Folder.h"

namespace model {

class AutoFolder
    :   public Folder
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /** Recovery constructor. */
    AutoFolder();

    /**
    * @param path full path to the folder.
    */
    AutoFolder(boost::filesystem::path path);

    ~AutoFolder();

    //////////////////////////////////////////////////////////////////////////
    // STRUCTURE
    //////////////////////////////////////////////////////////////////////////

    void update();

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    boost::filesystem::path getPath() const;
    wxString getName() const;
    boost::optional<wxString> getLastModified() const;

private:

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
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::AutoFolder, 1)
BOOST_CLASS_EXPORT(model::AutoFolder)

#endif // AUTO_FOLDER_H
