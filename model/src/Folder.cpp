#include "Folder.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "ProjectEventRenameAsset.h"
#include "UtilSerializeWxwidgets.h"
#include "GuiMain.h"
#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Folder::Folder()
:   mName()
{
    VAR_DEBUG(this);
}

Folder::Folder(wxString name)
:   mName(name)
{
    VAR_DEBUG(this)(mName);
}

Folder::~Folder()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET ATTRIBUTES
//////////////////////////////////////////////////////////////////////////

wxString Folder::getName() const
{
    return mName;
}

void Folder::setName(wxString name)
{
    if (name != mName)
    {
        mName = name;
        wxGetApp().QueueEvent(new ProjectEventRenameAsset(PROJECT_EVENT_RENAME_ASSET,shared_from_this(),mName));
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Folder::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<AProjectViewNode>(*this);
    ar & mName;
}
template void Folder::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Folder::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
