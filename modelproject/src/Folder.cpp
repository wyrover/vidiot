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
#include "Project.h"
#include "Window.h"
#include "UtilLog.h"
#include "UtilSerializeWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// ROOT
//////////////////////////////////////////////////////////////////////////

struct Root : public Folder
{
    Root() : Folder("Root") {};
    virtual ~Root() {};
    wxString getName() const override
    {
        return wxFileName(Project::get().getName()).GetName(); // strip extension
    }
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        try
        {
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Folder);
        }
        catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
        catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
        catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
        catch (...)                                  { LOG_ERROR;                                   throw; }
    }
};

// static
FolderPtr Folder::makeRoot()
{
    FolderPtr root;
    root.reset(new Root());
    return root;
}

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
    for ( model::NodePtr node : getChildren() )
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
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Node);
        ar & BOOST_SERIALIZATION_NVP(mName);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Folder::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Folder::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

template void Root::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Root::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);
} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Folder)

BOOST_CLASS_VERSION(model::Root, 1)
BOOST_CLASS_EXPORT_KEY(model::Root)
BOOST_CLASS_EXPORT_IMPLEMENT(model::Root)