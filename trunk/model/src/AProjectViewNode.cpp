#include "AProjectViewNode.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "GuiMain.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

DEFINE_EVENT(EVENT_ADD_ASSET,       EventAddAsset,      ParentAndChild);
DEFINE_EVENT(EVENT_REMOVE_ASSET,    EventRemoveAsset,   ParentAndChild);
DEFINE_EVENT(EVENT_RENAME_ASSET,    EventRenameAsset,   NodeWithNewName);

//////////////////////////////////////////////////////////////////////////
// CLASS
//////////////////////////////////////////////////////////////////////////

AProjectViewNode::AProjectViewNode()
:   mParent()
,   mChildren()
{
    VAR_DEBUG(this);
}

AProjectViewNode::~AProjectViewNode()
{
    VAR_DEBUG(*this);
}

AProjectViewNode::AProjectViewNode(const AProjectViewNode& other)
:   mParent()
,   mChildren()
{
}

//////////////////////////////////////////////////////////////////////////
// IDS
//////////////////////////////////////////////////////////////////////////

ProjectViewId AProjectViewNode::id()
{
    return static_cast<ProjectViewId>(this);
}

ProjectViewPtr AProjectViewNode::Ptr( ProjectViewId id )
{
    return id->shared_from_this();
}

//////////////////////////////////////////////////////////////////////////
// SET/GET STRUCTURE
//////////////////////////////////////////////////////////////////////////

bool AProjectViewNode::hasParent() const
{
    if (getParent())
    {
        return true;
    }
    else
    {
        return false;
    }
}

ProjectViewPtr AProjectViewNode::getParent() const
{ 
    return mParent.lock(); 
}

void AProjectViewNode::setParent(ProjectViewPtr parent)
{
    mParent = parent;
}

ProjectViewPtr AProjectViewNode::addChild(ProjectViewPtr newChild)
{
    mChildren.push_back(newChild);
    newChild->setParent(shared_from_this());
    // Do not use ProcessEvent: this will cause problems with auto-updating autofolders upon
    // first expansion.
    gui::wxGetApp().QueueEvent(new model::EventAddAsset(ParentAndChild(shared_from_this(),newChild)));
    return newChild;
}

ProjectViewPtr AProjectViewNode::removeChild(ProjectViewPtr child)
{
    VAR_DEBUG(this)(child.get());
    ProjectViewPtrs::iterator it;
    for (it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (*it == child) break;
    }
    ASSERT(it != mChildren.end());
    ProjectViewPtr p = *it;
    // Do not use ProcessEvent: see addChild
    gui::wxGetApp().QueueEvent(new model::EventRemoveAsset(ParentAndChild(shared_from_this(),child)));
    mChildren.erase(it);
    child->setParent(ProjectViewPtr());
    return p;
}

ProjectViewPtrs AProjectViewNode::getChildren() const
{
    return mChildren;
}

void AProjectViewNode::setName(wxString name)
{
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AProjectViewNode& obj )
{
    os << &obj << '|' << obj.mParent.lock() << '|' << obj.mChildren.size();
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AProjectViewNode::serialize(Archive & ar, const unsigned int version)
{
    if (Archive::is_loading::value)
    {
        ProjectViewPtr parent;
        ar & parent;
        setParent(parent);
    }
    else
    {
        ar & mParent.lock();
    }
    ar & mChildren;
}
template void AProjectViewNode::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AProjectViewNode::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace