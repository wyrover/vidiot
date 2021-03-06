// Copyright 2013-2016 Eric Raijmakers.
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

#include "INode.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

INode::INode()
{
};

INode::~INode()
{
};

//////////////////////////////////////////////////////////////////////////
// IDS
//////////////////////////////////////////////////////////////////////////

NodeId INode::id()
{
    return static_cast<NodeId>(this);
}

//static
NodePtr INode::Ptr(const NodeId& id)
{
    return id->self();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const INode& obj)
{
    os << &obj << '|' << obj.getName() << '|' << obj.getParent() << '|' << obj.getChildren().size();
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void INode::serialize(Archive & ar, const unsigned int version)
{
}

template void INode::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void INode::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::INode)
