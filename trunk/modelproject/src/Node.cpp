// Copyright 2013 Eric Raijmakers.
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

#include "Node.h"

#include "NodeEvent.h"

#include "UtilList.h"
#include "UtilLog.h"
#include "Window.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Node::Node()
    :   INode()
    ,   mParent()
    ,   mChildren()
{
    VAR_DEBUG(this);
}

Node::~Node()
{
    VAR_DEBUG(*this);
}

Node::Node(const Node& other)
    :   INode()
    ,   mParent()
    ,   mChildren()
{
}

//////////////////////////////////////////////////////////////////////////
// INODE
//////////////////////////////////////////////////////////////////////////

bool Node::hasParent() const
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

NodePtr Node::getParent() const
{
    return mParent.lock();
}

void Node::setParent(NodePtr parent)
{
    mParent = parent;
}

NodePtr Node::addChild(NodePtr newChild)
{
    mChildren.push_back(newChild);
    newChild->setParent(self());
    // Do not use ProcessEvent: this will cause problems with auto-updating autofolders upon
    // first expansion.
    gui::Window::get().GetEventHandler()->QueueEvent(new model::EventAddNode(ParentAndChildren(self(),newChild)));
    return newChild;
}

NodePtrs Node::addChildren(NodePtrs children)
{
    UtilList<NodePtr>(mChildren).addElements(children, NodePtr());
    BOOST_FOREACH( NodePtr child, children )
    {
        child->setParent(self());
    }
    // Do not use ProcessEvent: see addChild
    gui::Window::get().QueueModelEvent(new model::EventAddNodes(ParentAndChildren(self(),children)));
    return children;
}

NodePtr Node::removeChild(NodePtr child)
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
    gui::Window::get().QueueModelEvent(new model::EventRemoveNode(ParentAndChildren(self(),child)));
    mChildren.erase(it);
    child->setParent(NodePtr());
    return p;
}

NodePtrs Node::removeChildren(NodePtrs children)
{
    // Can't use UtilList::removeElements since these children may be 'out of order'
    BOOST_FOREACH( NodePtr child, children )
    {
        NodePtrs::iterator it;
        for (it = mChildren.begin(); it != mChildren.end(); ++it)
        {
            if (*it == child) break;
        }
        ASSERT(it != mChildren.end());
        mChildren.erase(it);
        child->setParent(NodePtr());
    }

    // Do not use ProcessEvent: see addChild
    gui::Window::get().QueueModelEvent(new model::EventRemoveNodes(ParentAndChildren(self(),children)));
    return children;
}

NodePtrs Node::getChildren() const
{
    return mChildren;
}

void Node::setName(wxString name)
{
}

int Node::count() const
{
    int result = 1; // 'this'
    BOOST_FOREACH( NodePtr child, mChildren )
    {
        result += child->count();
    }
    return result;
}

NodePtrs Node::find(wxString name)
{
    NodePtrs result;
    wxString _name = getName();
    if (getName().IsSameAs(name))
    {
        result.push_back(self());
    }
    BOOST_FOREACH( NodePtr child, mChildren )
    {
        UtilList<NodePtr>(result).addElements(child->find(name), NodePtr());
    }
    return result;
}

NodePtrs Node::findPath(wxString path)
{
    NodePtrs result;
    BOOST_FOREACH( NodePtr child, mChildren )
    {
        UtilList<NodePtr>(result).addElements(child->findPath(path), NodePtr());
    }
    return result;
}

bool Node::mustBeWatched(wxString path)
{
    BOOST_FOREACH( NodePtr child, mChildren )
    {
        if (child->mustBeWatched(path))
        {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Node& obj )
{
    os << &obj << '|' << obj.mParent.lock() << '|' << obj.mChildren.size();
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Node::serialize(Archive & ar, const unsigned int version)
{
    try
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
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Node::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Node::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace