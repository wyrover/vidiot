#include "AutoFolder.h"

#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <wx/dir.h>
#include <wx/filename.h>
#include "File.h"
#include "UtilList.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilLogWxwidgets.h"
#include "UtilLogStl.h"

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

AutoFolder::AutoFolder(wxFileName path)
:   Folder(util::path::toName(path))
,   mPath(util::path::normalize(path))
{
    ASSERT(path.IsDir())(path);
    VAR_DEBUG(this);
}

AutoFolder::~AutoFolder()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IPATH
//////////////////////////////////////////////////////////////////////////

wxFileName AutoFolder::getPath() const
{
    return mPath;
}

//////////////////////////////////////////////////////////////////////////
// STRUCTURE
//////////////////////////////////////////////////////////////////////////

// static
IPaths AutoFolder::getSupportedFiles( wxFileName directory )
{
    ASSERT( directory.IsDir() && directory.IsAbsolute() );
    IPaths result;
    wxDir dir( directory.GetLongPath() );
    ASSERT( dir.IsOpened() );
    wxString path;
    for (bool cont = dir.GetFirst(&path); cont; cont = dir.GetNext(&path))
    {
        wxFileName filename(directory.GetLongPath(), path);
        if (filename.IsDir())
        {
            AutoFolderPtr folder = boost::make_shared<AutoFolder>(filename);
            result.push_back(folder);
        }
        else
        {
            model::FilePtr file = boost::make_shared<model::File>(filename);
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
    // Fill 'allnames' with current list of children.
    std::list<wxString> allnames;
    BOOST_FOREACH( NodePtr child, getChildren() )
    {
        allnames.push_back(child->getName());
    }

    // Add nodes if required. Update 'allnames' if file name still present.
    BOOST_FOREACH( IPathPtr node, getSupportedFiles(mPath) )
    {
        wxString nodename = util::path::toName(node->getPath());
        if (UtilList<wxString>(allnames).hasElement(nodename))
        {
            // Existing element. Do not remove.
            UtilList<wxString>(allnames).removeElements(boost::assign::list_of(nodename));
        }
        else
        {
            // New element. Add.
            addChild(boost::dynamic_pointer_cast<AProjectViewNode>(node));
        }
    }

    // Remove all other elements (these have been removed).
    BOOST_FOREACH( wxString name, allnames )
    {
        BOOST_FOREACH( NodePtr child, getChildren() )
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

wxString AutoFolder::getName() const
{
    if (getParent()->isA<AutoFolder>())
    {
        return util::path::toName(mPath);
    }
    return mPath.GetFullPath();
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

