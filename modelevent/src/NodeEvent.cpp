#include "NodeEvent.h"

#include "INode.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogBoost.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_NODE,       EventAddNode,      ParentAndChildren);
DEFINE_EVENT(EVENT_ADD_NODES,      EventAddNodes,     ParentAndChildren);
DEFINE_EVENT(EVENT_REMOVE_NODE,    EventRemoveNode,   ParentAndChildren);
DEFINE_EVENT(EVENT_REMOVE_NODES,   EventRemoveNodes,  ParentAndChildren);
DEFINE_EVENT(EVENT_RENAME_NODE,    EventRenameNode,   NodeWithNewName);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ParentAndChildren::ParentAndChildren( NodePtr parent, NodePtrs children )
    :  mParent(parent)
    ,  mChildren(children)
{
}

ParentAndChildren::ParentAndChildren( NodePtr parent, NodePtr child )
    :  mParent(parent)
    ,  mChildren()
{
    mChildren.push_back(child);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

NodePtr ParentAndChildren::getParent() const
{
    return mParent;
}

NodePtrs ParentAndChildren::getChildren() const
{
    return mChildren;
}

NodePtr ParentAndChildren::getChild() const
{
    ASSERT_EQUALS(mChildren.size(),1);
    return mChildren.front();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ParentAndChildren& obj )
{
    os << &obj << '|' << obj.mParent << '|' << obj.mChildren;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

NodeWithNewName::NodeWithNewName( NodePtr node, wxString name )
    :   mNode(node)
    ,   mName(name)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

NodePtr NodeWithNewName::getNode() const
{
    return mNode;
}

wxString NodeWithNewName::getName() const
{
    return mName;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const NodeWithNewName& obj )
{
    os << &obj << '|' << obj.mNode << '|' << obj.mName;
    return os;
}

} // namespace