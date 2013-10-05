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

#include "Project.h"

#include "CommandProcessor.h"
#include "Dialog.h"
#include "File.h"
#include "Folder.h"
#include "IView.h"
#include "ProjectEvent.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilLog.h"

namespace model {

IMPLEMENT_DYNAMIC_CLASS(Project, wxDocument)

Project::Project()
:   wxDocument()
// Do not initialize members with actual data here.
// For loading that is done via serialize - Initializing here for loading is useless (overwritten by serialize) and causes crashes (mProperties instantiated twice)
// For new documents initializing is done via OnNewDocument
,   mRoot(boost::make_shared<Folder>("Root")) // Exception: Initialized here since it is used on OnChangeFilename which is called before any other method when creating a new project.
,   mProperties()
{
    VAR_DEBUG(this);
    ASSERT(!IsModified());
}

Project::~Project()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// OVERRIDES - wxDocument
//////////////////////////////////////////////////////////////////////////

bool Project::DeleteContents()
{
    VAR_DEBUG(this);
    GetCommandProcessor()->ClearCommands();
    return wxDocument::DeleteContents();
}

bool Project::OnCloseDocument()
{
    // Uses ProcessEvent explicitly. The events must be handled immediately,
    // because wxWidgets will destruct this Project object directly after
    // calling OnCloseDocument(). If QueueEvent is used, the event is handled
    // AFTER the destruction of this object which leads to crashes.

    EventCloseProject closeEvent(this); // Do not 'inline' in the next line like IView::get().ProcessModelEvent(EventCloseProject(this)); Doesn't compile in g++
    IView::getView().ProcessModelEvent(closeEvent);
    return wxDocument::OnCloseDocument();
}

bool Project::OnNewDocument()
{
    bool opened = wxDocument::OnNewDocument();
    if (opened)
    {
        mProperties = boost::make_shared<Properties>();
        IView::getView().ProcessModelEvent(EventOpenProject(this));
    }
    return opened;
}

bool Project::OnCreate(const wxString& path, long flags)
{
    bool created = wxDocument::OnCreate(path,flags);
    if (created)
    {
        GetCommandProcessor()->Initialize();
    }
    return created;
}

wxCommandProcessor* Project::OnCreateCommandProcessor()
{
    return new CommandProcessor();
}

void Project::OnChangeFilename(bool notifyViews)
{
    mRoot->setName(GetUserReadableName());
    IView::getView().ProcessModelEvent(EventRenameProject(this));
    wxDocument::OnChangeFilename(notifyViews);
}

//////////////////////////////////////////////////////////////////////////
// LOAD/SAVE
//////////////////////////////////////////////////////////////////////////

const std::string sProject("project");
const std::string sView("view");

std::ostream& Project::SaveObject(std::ostream& ostream)
{
    gui::StatusBar::get().pushInfoText(_("Saving ") + mRoot->getName() + _(" ..."));
    try
    {
        boost::archive::xml_oarchive ar(ostream);
        ar & boost::serialization::make_nvp(sProject.c_str(),*this);
        ar & boost::serialization::make_nvp(sView.c_str(),IView::getView());
    }
    catch (boost::archive::archive_exception& e)
    {
        FATAL(e.what());
    }
    catch (boost::exception &e)
    {
        FATAL(boost::diagnostic_information(e));
    }
    catch (std::exception& e)
    {
        FATAL(e.what());
    }
    catch (...)
    {
        FATAL;
    }
    gui::StatusBar::get().popInfoText();
    gui::StatusBar::get().timedInfoText(mRoot->getName() + _(" saved successfully."));
    return ostream;
}

std::istream& Project::LoadObject(std::istream& istream)
{
    try
    {
        boost::archive::xml_iarchive ar(istream);
        ar & boost::serialization::make_nvp(sProject.c_str(),*this);
        ar & boost::serialization::make_nvp(sView.c_str(),IView::getView());
        IView::getView().ProcessModelEvent(EventOpenProject(this));
        mRoot->check();
    }
    catch (boost::archive::archive_exception& e)
    {
        VAR_ERROR(e.what());
        istream.setstate(std::ios_base::failbit);
    }
    catch (boost::exception &e)
    {
        VAR_ERROR(boost::diagnostic_information(e));
        istream.setstate(std::ios_base::failbit);
    }
    catch (std::exception& e)
    {
        VAR_ERROR(e.what());
        istream.setstate(std::ios_base::failbit);
    }
    catch (...)
    {
        LOG_ERROR;
        istream.setstate(std::ios_base::failbit);
    }
    // NOT: FATAL("Could not load");
    return istream;
}

bool Project::DoOpenDocument(const wxString& file)
{
    std::ifstream store(file.mb_str(), wxSTD ios::binary);
    if ( !store )
    {
        gui::Dialog::get().getConfirmation(_("Open Failed"),_("Could not open: " + file));
    }
    else
    {
        LoadObject(store);
        if ( !store )
        {
            // The bug is in 'mProperties' having a use count of '2' at this point:
            // Memory leak of Properties. Causes crash when opening a new project.
            // ASSERT(mProperties.unique());
            LOG_ERROR;
            gui::Dialog::get().getConfirmation(_("Open Failed"),_("Could not read the contents of: " + file + ". \nVidiot must be restarted ((known bug that opening a project after this will fail)"));
            gui::Window::get().GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,wxID_EXIT));
        }
        else
        {
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
// ACCESSORS
//////////////////////////////////////////////////////////////////////////

FolderPtr Project::getRoot() const
{
    return mRoot;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Project::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        // Since the properties can be used by other objects, they must be read first.
        // An example is the framerate, which is used by 'Convert' which, in turn, is used in openFile() to determine the length of a stream in the file.
        ar & BOOST_SERIALIZATION_NVP(mProperties);
        ar & BOOST_SERIALIZATION_NVP(mRoot);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Project::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Project::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Project)