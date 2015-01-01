// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "NodeEvent.h"

#include "INode.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_NODE,       EventAddNode,      ParentAndChildren);
DEFINE_EVENT(EVENT_ADD_NODES,      EventAddNodes,     ParentAndChildren);
DEFINE_EVENT(EVENT_REMOVE_NODE,    EventRemoveNode,   ParentAndChildren);
DEFINE_EVENT(EVENT_REMOVE_NODES,   EventRemoveNodes,  ParentAndChildren);
DEFINE_EVENT(EVENT_RENAME_NODE,    EventRenameNode,   NodeWithNewName);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ParentAndChildren::ParentAndChildren(const NodePtr& parent, const NodePtrs& children )
    :  mParent(parent)
    ,  mChildren(children)
{
}

ParentAndChildren::ParentAndChildren(const NodePtr& parent, const NodePtr& child )
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

std::ostream& operator<<(std::ostream& os, const ParentAndChildren& obj)
{
    os << &obj << '|' << obj.mParent << '|' << obj.mChildren;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

NodeWithNewName::NodeWithNewName(const NodePtr& node, const wxString& name )
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

std::ostream& operator<<(std::ostream& os, const NodeWithNewName& obj)
{
    os << &obj << '|' << obj.mNode << '|' << obj.mName;
    return os;
}

} // namespace