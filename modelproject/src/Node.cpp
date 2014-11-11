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

#include "Node.h"

#include "NodeEvent.h"

#include "UtilLog.h"
#include "Window.h"
#include "UtilVector.h"

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

void Node::setParent(const NodePtr& parent)
{
    mParent = parent;
}

NodePtr Node::addChild(const NodePtr& newChild)
{
    mChildren.push_back(newChild);
    newChild->setParent(self());
    // Do not use ProcessEvent: this will cause problems with auto-updating autofolders upon
    // first expansion.
    gui::Window::get().GetEventHandler()->QueueEvent(new model::EventAddNode(ParentAndChildren(self(),newChild)));
    return newChild;
}

NodePtrs Node::addChildren(const NodePtrs& children)
{
    UtilVector<NodePtr>(mChildren).addElements(children, NodePtr());
    for ( NodePtr child : children )
    {
        child->setParent(self());
    }
    // Do not use ProcessEvent: see addChild
    gui::Window::get().QueueModelEvent(new model::EventAddNodes(ParentAndChildren(self(),children)));
    return children;
}

NodePtr Node::removeChild(const NodePtr& child)
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

NodePtrs Node::removeChildren(const NodePtrs& children)
{
    // Can't use UtilVector::removeElements since these children may be 'out of order'
    for ( NodePtr child : children )
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

NodePtrs Node::getAllDescendants() const
{
    NodePtrs result;
    UtilVector<NodePtr>(result).addElements(mChildren, NodePtr());
    for ( NodePtr child : mChildren )
    {
        UtilVector<NodePtr>(result).addElements(child->getAllDescendants(), NodePtr());
    }
    return result;
}

void Node::setName(const wxString& name)
{
}

int Node::count() const
{
    int result = 1; // 'this'
    for ( NodePtr child : mChildren )
    {
        result += child->count();
    }
    return result;
}

NodePtrs Node::find(const wxString& name)
{
    NodePtrs result;
    wxString _name = getName();
    if (getName().IsSameAs(name))
    {
        result.push_back(self());
    }
    for ( NodePtr child : mChildren )
    {
        UtilVector<NodePtr>(result).addElements(child->find(name), NodePtr());
    }
    return result;
}

NodePtrs Node::findPath(const wxString& path)
{
    NodePtrs result;
    for ( NodePtr child : mChildren )
    {
        UtilVector<NodePtr>(result).addElements(child->findPath(path), NodePtr());
    }
    return result;
}

bool Node::mustBeWatched(const wxString& path)
{
    for ( NodePtr child : mChildren )
    {
        if (child->mustBeWatched(path))
        {
            return true;
        }
    }
    return false;
}

bool Node::hasSequences() const
{
    for ( NodePtr child : mChildren )
    {
        if (child->hasSequences())
        {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Node& obj)
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
    const std::string sParent("parent");
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(INode);
        if (Archive::is_loading::value)
        {
            NodePtr parent;
            ar & boost::serialization::make_nvp(sParent.c_str(),parent);
            setParent(parent);
        }
        else
        {
            NodePtr parent = mParent.lock();
            ar & boost::serialization::make_nvp(sParent.c_str(),parent);
        }
        ar & BOOST_SERIALIZATION_NVP(mChildren);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Node::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Node::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Node)
