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

#include "Folder.h"

#include "NodeEvent.h"
#include "Window.h"
#include "UtilLog.h"
#include "UtilSerializeWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Folder::Folder()
:   mName()
{
    VAR_DEBUG(this);
}

Folder::Folder(wxString name)
:   mName(name)
{
    VAR_DEBUG(this)(mName);
}

Folder::~Folder()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// INODE
//////////////////////////////////////////////////////////////////////////

void Folder::check()
{
    BOOST_FOREACH( model::NodePtr node, getChildren() )
    {
        node->check();
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET ATTRIBUTES
//////////////////////////////////////////////////////////////////////////

wxString Folder::getName() const
{
    return mName;
}

void Folder::setName(wxString name)
{
    if (name != mName)
    {
        mName = name;
        gui::Window::get().QueueModelEvent(new EventRenameNode(NodeWithNewName(self(),mName)));
    }
}

wxString Folder::getSequenceName() const
{
    return mName;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Folder::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Node>(*this);
    ar & mName;
}
template void Folder::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Folder::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace