#include "AProjectViewNode.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "ProjectViewNodeEvent.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "Window.h"

namespace model {

// todo rename AProjectViewNode to Asset/Node

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AProjectViewNode::AProjectViewNode()
    :   INode()
    ,   mParent()
    ,   mChildren()
{
    VAR_DEBUG(this);
}

AProjectViewNode::~AProjectViewNode()
{
    VAR_DEBUG(*this);
}

AProjectViewNode::AProjectViewNode(const AProjectViewNode& other)
    :   INode()
    ,   mParent()
    ,   mChildren()
{
}

//////////////////////////////////////////////////////////////////////////
// INODE
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

NodePtr AProjectViewNode::getParent() const
{ 
    return mParent.lock(); 
}

void AProjectViewNode::setParent(NodePtr parent)
{
    mParent = parent;
}

NodePtr AProjectViewNode::addChild(NodePtr newChild)
{
    mChildren.push_back(newChild);
    newChild->setParent(shared_from_this());
    // Do not use ProcessEvent: this will cause problems with auto-updating autofolders upon
    // first expansion.
    gui::Window::get().GetEventHandler()->QueueEvent(new model::EventAddAsset(ParentAndChild(shared_from_this(),newChild)));
    return newChild;
}

NodePtr AProjectViewNode::removeChild(NodePtr child)
{
    VAR_DEBUG(this)(child.get());
    NodePtrs::iterator it;
    for (it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (*it == child) break;
    }
    ASSERT(it != mChildren.end());
    NodePtr p = *it;
    // Do not use ProcessEvent: see addChild
    gui::Window::get().QueueModelEvent(new model::EventRemoveAsset(ParentAndChild(shared_from_this(),child)));
    mChildren.erase(it);
    child->setParent(NodePtr());
    return p;
}

NodePtrs AProjectViewNode::getChildren() const
{
    return mChildren;
}

void AProjectViewNode::setName(wxString name)
{
}

NodePtrs AProjectViewNode::find(wxString name)
{
    NodePtrs result;
    wxString _name = getName();
    if (getName().IsSameAs(name))
    {
        result.push_back(shared_from_this());
    }
    BOOST_FOREACH( NodePtr child, mChildren )
    {
        UtilList<NodePtr>(result).addElements(child->find(name), NodePtr());
    }
    return result;
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
    ar & boost::serialization::base_object<INode>(*this);
    if (Archive::is_loading::value)
    {
        NodePtr parent;
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