#include "AProjectViewNode.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "ProjectEventAddAsset.h"
#include "ProjectEventDeleteAsset.h"
#include "UtilLog.h"
#include "GuiMain.h"

namespace model {

AProjectViewNode::AProjectViewNode()
:   mParent()
,   mChildren()
{
    VAR_DEBUG(this);
}

AProjectViewNode::~AProjectViewNode()
{
    VAR_DEBUG(this);
    ASSERT(mChildren.size() == 0)(shared_from_this())(mChildren.size());
}

void AProjectViewNode::Delete()
{
    // First 'bottom up' reference removal,
    // Second 'top down' reference removal.
    while (mChildren.size() > 0)
    {
        ProjectViewPtr child = *mChildren.begin();
        child ->Delete();
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
    wxGetApp().QueueEvent(new ProjectEventAddAsset(PROJECT_EVENT_ADD_ASSET,shared_from_this(),newChild));
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
    wxGetApp().QueueEvent(new ProjectEventDeleteAsset(PROJECT_EVENT_DELETE_ASSET,shared_from_this(),child));
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