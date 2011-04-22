#include "AutoFolder.h"
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "File.h"
#include "UtilList.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilLogWxwidgets.h"
#include "UtilLogStl.h"
#include "FSWatcher.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AutoFolder::AutoFolder()
:   Folder()
,   mPath()
{
    VAR_DEBUG(this);
}

AutoFolder::AutoFolder(boost::filesystem::path path)
:   Folder(path.filename().string())
,   mPath(path)
{
    VAR_DEBUG(this);
    gui::FSWatcher::current()->watchFolder(this);
}

AutoFolder::~AutoFolder()
{
    VAR_DEBUG(this);
    gui::FSWatcher::current()->unwatchFolder(this);
}

//////////////////////////////////////////////////////////////////////////
// STRUCTURE
//////////////////////////////////////////////////////////////////////////

void AutoFolder::update()
{
    std::list<wxString> allnames;
    BOOST_FOREACH( ProjectViewPtr child, getChildren() )
    {
        allnames.push_back(child->getName());
    }

    for (boost::filesystem::directory_iterator itr(mPath); itr != boost::filesystem::directory_iterator(); ++itr)
    {
        std::string leaf = itr->path().filename().string();
        wxString name(leaf);
        bool isDir(is_directory(*itr));
        bool isFile(is_regular_file(*itr));

        if (UtilList<wxString>(allnames).hasElement(name))
        {
            UtilList<wxString>(allnames).removeElements(boost::assign::list_of(name));
            continue;
        }
        if (isDir)
        {
            AutoFolderPtr folder = boost::make_shared<AutoFolder>(mPath / leaf);
            addChild(folder);
        }
        else if (isFile)
        {
            model::FilePtr file = boost::make_shared<model::File>(mPath  / leaf);
            if (file->isSupported())
            {
                addChild(file);
            }
        }
    }
    // Remove all other nodes
    BOOST_FOREACH( wxString name, allnames )
    {
        BOOST_FOREACH( ProjectViewPtr child, getChildren() )
        {
            if (child->getName().CompareTo(name) == 0)
            {
                removeChild(child);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// ATTRIBUTES
//////////////////////////////////////////////////////////////////////////

wxFileName AutoFolder::getFileName() const
{
    return wxFileName(mPath.string(),"");
}

boost::filesystem::path AutoFolder::getPath() const
{
    return mPath;
}

wxString AutoFolder::getName() const
{
    return mPath.filename().string();
}

boost::optional<wxString> AutoFolder::getLastModified() const
{
    return mLastModified;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AutoFolder::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Folder>(*this);
    ar & mPath;
    ar & mLastModified;
    if (Archive::is_loading::value)
    {
        gui::FSWatcher::current()->watchFolder(this);
    }
}
template void AutoFolder::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AutoFolder::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace

