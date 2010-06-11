#include "AutoFolder.h"
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "File.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"

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
:   Folder(path.leaf())
,   mPath(path)
{
    VAR_DEBUG(this);
}

AutoFolder::~AutoFolder()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// STRUCTURE
//////////////////////////////////////////////////////////////////////////

void AutoFolder::update()
{
    for (boost::filesystem::directory_iterator itr(mPath); itr != boost::filesystem::directory_iterator(); ++itr)
    {
        std::string leaf = itr->path().leaf();

        bool exists = false;
        BOOST_FOREACH( ProjectViewPtr existingchild, getChildren() )
        {
            if (existingchild->getName().IsSameAs(leaf))
            {
                exists = true;
                break;
            }
        }
        if (exists)
        {
            continue;
        }

        if (is_directory(*itr))
        {
            AutoFolderPtr folder = boost::make_shared<AutoFolder>(mPath / leaf);
            addChild(folder);
        }
        else if (is_regular_file(*itr))
        {
            model::FilePtr file = boost::make_shared<model::File>(mPath  / leaf);
            if (file->isSupported())
            {
                addChild(file);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// ATTRIBUTES
//////////////////////////////////////////////////////////////////////////

boost::filesystem::path AutoFolder::getPath() const
{
    return mPath;
}

wxString AutoFolder::getName() const
{
    return mPath.leaf();
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
}
template void AutoFolder::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AutoFolder::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
