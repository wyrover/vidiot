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

#include "Project.h"

#include "CommandProcessor.h"
#include "Config.h"
#include "Dialog.h"
#include "File.h"
#include "Folder.h"
#include "IView.h"
#include "DialogNewProject.h"
#include "ProjectEvent.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilLog.h"
#include "UtilRecycle.h"

namespace model {

IMPLEMENT_DYNAMIC_CLASS(Project, wxDocument)

Project::Project()
:   wxDocument()
// Do not initialize members with actual data here.
// For loading that is done via serialize - Initializing here for loading is useless (overwritten by serialize) and causes crashes (mProperties instantiated twice)
// For new documents initializing is done via OnNewDocument
,   mRoot(Folder::makeRoot()) // Exception: Initialized here since it is used on OnChangeFilename which is called before any other method when creating a new project.
,   mProperties()
,   mSaveFolder("")
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
    //gui::DialogNewProject* dialog = new gui::DialogNewProject(this);
    //dialog->ShowModal();
    if (opened)
    {
        mProperties = boost::make_shared<Properties>();
        EventOpenProject event(true);
        IView::getView().ProcessModelEvent(event);
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
    wxDocument::OnChangeFilename(notifyViews);
    EventRenameProject event(this);
    IView::getView().ProcessModelEvent(event);
    gui::Window::get().QueueModelEvent(new EventRenameNode(NodeWithNewName(mRoot,mRoot->getName())));
}

//////////////////////////////////////////////////////////////////////////
// LOAD/SAVE
//////////////////////////////////////////////////////////////////////////

const std::string sProject("project");
const std::string sView("view");

std::ostream& Project::SaveObject(std::ostream& ostream)
{
    bool ok = false;
    try
    {
        boost::archive::xml_oarchive ar(ostream);
        ar & boost::serialization::make_nvp(sProject.c_str(),*this);
        ar & boost::serialization::make_nvp(sView.c_str(),IView::getView());
        ok = true;
    }
    catch (boost::archive::archive_exception& e)
    {
        VAR_ERROR(e.what());
    }
    catch (boost::exception &e)
    {
        VAR_ERROR(boost::diagnostic_information(e));
    }
    catch (std::exception& e)
    {
        VAR_ERROR(e.what());
    }
    catch (...)
    {
        LOG_ERROR;
    }
    if (!ok)
    {
        ostream.setstate(std::ios_base::failbit);
    }
    return ostream;
}

std::istream& Project::LoadObject(std::istream& istream)
{
    try
    {
        boost::archive::xml_iarchive ar(istream);
        ar & boost::serialization::make_nvp(sProject.c_str(),*this);
        ar & boost::serialization::make_nvp(sView.c_str(),IView::getView());
        EventOpenProject event(false);
        IView::getView().ProcessModelEvent(event);
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

// static
wxFileName Project::createBackupFileName(wxFileName input, int count)
{
    wxString name = input.GetName();
    name.Append(wxString::Format("_%d", count));
    input.SetName(name);
    return input;
}

bool Project::DoSaveDocument(const wxString& file)
{
    wxFileName saveFileName(file);
    wxFileName saveFolder(file);
    saveFolder.SetFullName(""); // Remove name and ext
    mSaveFolder = util::path::normalize(saveFolder).GetLongPath();
    gui::StatusBar::get().pushInfoText(_("Saving ") + saveFileName.GetFullName() + _(" ..."));
    if (saveFileName.Exists() &&
        Config::ReadBool(Config::sPathBackupBeforeSaveEnabled))
    {
        // Find all existing backup files
        wxArrayString existingBackupFiles;
        wxString pattern; pattern << saveFileName.GetName() << "_*" << saveFileName.GetExt();
        long nExistingBackupFiles = wxDir::GetAllFiles(saveFileName.GetPath(), &existingBackupFiles, pattern, wxDIR_FILES | wxDIR_NO_FOLLOW);

        // Find the one with the highest and the one with the lowest number
        long nextFreeNumber = 0;
        long lowestNumber = std::numeric_limits<long>::max();
        for (wxString file : existingBackupFiles)
        {
            wxFileName fileName(file);
            wxString stripped = fileName.GetName();
            if (stripped.Replace(saveFileName.GetName() + "_","") == 1) // else: no vidiot generated save file
            {
                long number = 0;
                if (stripped.ToLong(&number)) // else: no vidiot generated save file
                {
                    nextFreeNumber = std::max(nextFreeNumber,number + 1);
                    lowestNumber = std::min(lowestNumber,number);
                }
            }
        }

        // Create the new backup file
        wxFileName backup = createBackupFileName(saveFileName, nextFreeNumber);
        ASSERT(!backup.Exists());
        if (!wxCopyFile(saveFileName.GetFullPath(), backup.GetFullPath(),false) &&
            wxNO == gui::Dialog::get().getConfirmation(_("Backup failed"),_("Could not create backup file " + backup.GetFullName() + " of the existing save.\n Do you still want to overwrite " + file + "?"), wxYES | wxNO ))
        {
            return true;
        }

        // If configured, remove a backup file to ensure that the number of backups will not exceed the maximum
        // Note: if for some reason the number of backups exceeds the maximum then still only one is deleted (no goldplating...).
        int maximumNumberOfFiles = Config::ReadLong(Config::sPathBackupBeforeSaveMaximum);
        if (maximumNumberOfFiles > 0 && nExistingBackupFiles >= maximumNumberOfFiles)
        {
            wxFileName backup = createBackupFileName(saveFileName, lowestNumber);
            if (!backup.Exists() || !util::path::recycle(backup.GetFullPath()))
            {
                VAR_ERROR(backup.Exists())(backup.GetFullPath());
            }
        }
    }

    // Do the actual save
    std::ofstream store(file.mb_str(), wxSTD ios::binary);
    if ( !store )
    {
        gui::StatusBar::get().popInfoText();
        gui::Dialog::get().getConfirmation(_("Save Failed"),_("Could not open save file: " + file));
    }
    else
    {
        if (!SaveObject(store))
        {
            gui::StatusBar::get().popInfoText();
            gui::Dialog::get().getConfirmation(_("Save Failed"),_("Could not save: " + file));
        }
        else
        {
            gui::StatusBar::get().popInfoText();
            gui::StatusBar::get().timedInfoText(saveFileName.GetFullName() + _(" saved successfully."));
        }
    }
    return true;
}

bool Project::DoOpenDocument(const wxString& file)
{
    wxFileName saveFolder(file);
    saveFolder.SetFullName(""); // Remove name and ext
    mSaveFolder = util::path::normalize(saveFolder).GetLongPath();
    std::ifstream store(file.mb_str(), wxSTD ios::binary);
    if ( !store )
    {
        gui::Dialog::get().getConfirmation(_("Open Failed"),_("Could not open: " + file));
    }
    else
    {
        if ( !LoadObject(store) )
        {
            // The bug is in 'mProperties' having a use count of '2' at this point:
            // Memory leak of Properties. Causes crash when opening a new project.
            // ASSERT(mProperties.unique());
            LOG_ERROR;
            gui::Dialog::get().getConfirmation(_("Open Failed"),_("Could not read the contents of: " + file + ". \nVidiot must be restarted ((known bug that opening a project after this will fail)"));
            Config::WriteBool(Config::sPathAutoLoadEnabled, false); // Ensure that upon next startup not immediately a file is opened, possibly failing again.
            wxConfigBase::Get()->Flush();
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

wxString Project::getName() const
{
    return GetUserReadableName();
}

wxFileName Project::convertPathForSaving(const wxFileName& path) const
{
    ASSERT(path.IsAbsolute());
    wxFileName result(path);
    if (Config::ReadLong(Config::sPathSavePathsRelativeToProject))
    {
        result.MakeRelativeTo(mSaveFolder);
    }
    return result;
}

wxFileName Project::convertPathAfterLoading(const wxFileName& path) const
{
    wxFileName result(path);
    if (result.IsRelative())
    {
        result.MakeAbsolute(mSaveFolder);
    }
    return result;
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
