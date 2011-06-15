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

// static
model::ProjectViewPtrs AutoFolder::getSupportedFiles( boost::filesystem::path directory )
{
    model::ProjectViewPtrs result;
    for (boost::filesystem::directory_iterator itr(directory); itr != boost::filesystem::directory_iterator(); ++itr)
    {
        if (is_directory(*itr))
        {
            AutoFolderPtr folder = boost::make_shared<AutoFolder>(itr->path());
            result.push_back(folder);
        }
        else if (is_regular_file(*itr))
        {
            model::FilePtr file = boost::make_shared<model::File>(itr->path());
            if (file->isSupported())
            {
                result.push_back(file);
            }
        }
    }
    return result;
}

void AutoFolder::update()
{
    std::list<wxString> allnames;
    BOOST_FOREACH( ProjectViewPtr child, getChildren() )
    {
        allnames.push_back(child->getName());
    }

    BOOST_FOREACH( model::ProjectViewPtr asset, getSupportedFiles(mPath) )
    {
        if (UtilList<wxString>(allnames).hasElement(asset->getName()))
        {
            // Existing element. Do not remove.
            UtilList<wxString>(allnames).removeElements(boost::assign::list_of(asset->getName()));
        }
        else
        {
            // New element. Add.
            addChild(asset);
        }
    }
    // Remove all other elements (these have been removed)
    BOOST_FOREACH( wxString name, allnames )
    {
        BOOST_FOREACH( ProjectViewPtr child, getChildren() )
        {
            if (!child->getName().IsSameAs(name))
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

