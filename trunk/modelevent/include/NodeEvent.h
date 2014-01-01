// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef PROJECT_VIEW_NODE_EVENT_H
#define PROJECT_VIEW_NODE_EVENT_H

#include "UtilEvent.h"

namespace model {

class ParentAndChildren
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ParentAndChildren(NodePtr parent, NodePtrs children);
    ParentAndChildren(NodePtr parent, NodePtr child);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    NodePtr getParent() const;
    NodePtrs getChildren() const;
    NodePtr getChild() const; ///< May only be called for the events that indicate that 1 and only one child has been changed

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    NodePtr mParent;
    NodePtrs mChildren;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ParentAndChildren& obj );

};

DECLARE_EVENT(EVENT_ADD_NODE,      EventAddNode,      ParentAndChildren);
DECLARE_EVENT(EVENT_ADD_NODES,     EventAddNodes,     ParentAndChildren);
DECLARE_EVENT(EVENT_REMOVE_NODE,   EventRemoveNode,   ParentAndChildren);
DECLARE_EVENT(EVENT_REMOVE_NODES,  EventRemoveNodes,  ParentAndChildren);

class NodeWithNewName
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    NodeWithNewName( NodePtr node, wxString name);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    NodePtr getNode() const;
    wxString getName() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    NodePtr mNode;
    wxString mName;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const NodeWithNewName& obj );

};

DECLARE_EVENT(EVENT_RENAME_NODE,   EventRenameNode,   NodeWithNewName);

} // namespace

#endif // PROJECT_VIEW_NODE_EVENT_H