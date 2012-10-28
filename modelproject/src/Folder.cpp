#include "Folder.h"

#include "NodeEvent.h"
#include "Window.h"
#include "UtilLog.h"
#include "UtilSerializeWxwidgets.h"

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
        gui::Window::get().QueueModelEvent(new EventRenameNode(NodeWithNewName(shared_from_this(),mName)));
    }
}

wxString Folder::getSequenceName() const
{
    return mName;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Folder::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Node>(*this);
    ar & mName;
}
template void Folder::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Folder::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace