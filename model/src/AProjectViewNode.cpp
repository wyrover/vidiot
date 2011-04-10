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
    ASSERT(mChildren.size() == 0)(this)(mChildren.size());
}

AProjectViewNode::AProjectViewNode(const AProjectViewNode& other)
:   mParent()
,   mChildren()
{
}

void AProjectViewNode::destroy()
{
    // First 'bottom up' reference removal,
    // Second 'top down' reference removal.
    while (mChildren.size() > 0)
    {
        ProjectViewPtr child = *mChildren.begin();
        child->destroy();
        removeChild(child);
    }
    mParent.reset();
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
    if (mParent)
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
    return mParent; 
}

void AProjectViewNode::setParent(ProjectViewPtr parent)
{
    mParent = parent;
}

ProjectViewPtr AProjectViewNode::addChild(ProjectViewPtr newChild)
{
    mChildren.push_back(newChild);
    newChild->setParent(shared_from_this());
    // The event must be handled immediately due to the use of shared_ptr
    // (more important in case of deletion than in case of addition)
    gui::wxGetApp().ProcessEvent(model::EventAddAsset(ParentAndChild(shared_from_this(),newChild)));
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
    // The event must be handled immediately due to the use of shared_ptr
    gui::wxGetApp().ProcessEvent(model::EventRemoveAsset(ParentAndChild(shared_from_this(),child)));
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
    os << &obj << '|' << obj.mParent << '|' << obj.mChildren.size();
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AProjectViewNode::serialize(Archive & ar, const unsigned int version)
{
    ar & mParent;
    ar & mChildren;
}
template void AProjectViewNode::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AProjectViewNode::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace