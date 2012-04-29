#include "Project.h"

#include <fstream>
#include <wx/msgdlg.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "CommandProcessor.h"
#include "Dialog.h"
#include "File.h"
#include "Folder.h"
#include "IView.h"
#include "ProjectEvent.h"
#include "Properties.h"
#include "Serialization.h"
#include "UtilLog.h"

namespace model {

IMPLEMENT_DYNAMIC_CLASS(Project, wxDocument)

static Project* sCurrent = 0;

Project::Project()
:   wxDocument()
,   mRoot(boost::make_shared<Folder>("Root"))
,   mProperties(boost::make_shared<Properties>())
{
    VAR_DEBUG(this);
    sCurrent = this;
    ASSERT(!IsModified());
}

Project::~Project()
{
    VAR_DEBUG(this);
    sCurrent = 0;
}

Project& Project::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
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
    IView::get().ProcessModelEvent(closeEvent);
    return wxDocument::OnCloseDocument();
}

bool Project::OnNewDocument()
{
    bool opened = wxDocument::OnNewDocument();
    if (opened)
    {
        IView::get().ProcessModelEvent(EventOpenProject(this));
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
    IView::get().ProcessModelEvent(EventRenameProject(this));
    wxDocument::OnChangeFilename(notifyViews);
}

void Project::Modify(bool modify)
{
    wxDocument::Modify(modify);
}

//////////////////////////////////////////////////////////////////////////
// LOAD/SAVE
//////////////////////////////////////////////////////////////////////////

std::ostream& Project::SaveObject(std::ostream& ostream)
{
    try
    {
        boost::archive::text_oarchive ar(ostream);
        registerClasses(ar);
        ar & *this;
        ar & IView::get();
    }
    catch (boost::archive::archive_exception& e)
    {
        FATAL(e.what());
    }
    catch (std::exception& e)
    {
        FATAL(e.what());
    }
    catch (...)
    {
        FATAL;
    }
    return ostream;
}

std::istream& Project::LoadObject(std::istream& istream)
{
    try
    {
        boost::archive::text_iarchive ar(istream);
        registerClasses(ar);
        ar & *this;
        ar & IView::get();
        IView::get().ProcessModelEvent(EventOpenProject(this));
    }
    catch (boost::archive::archive_exception& e)
    {
        VAR_ERROR(e.what());
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

    return istream;
}

bool Project::DoOpenDocument(const wxString& file)
{
    std::ifstream store(file.mb_str(), wxSTD ios::binary);
    if ( !store )
    {
        gui::Dialog::get().getConfirmation(_("Open Failed"),_("The file could not be opened."));
        return false;
    }
    LoadObject(store);
    if ( !store )
    {
        gui::Dialog::get().getConfirmation(_("Open Failed"),_("The file contents could not be read."));
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// CHANGE COMMANDS
//////////////////////////////////////////////////////////////////////////

void Project::Submit(command::RootCommand* c)
{
    Modify(true);
    GetCommandProcessor()->Submit(c);
}

//////////////////////////////////////////////////////////////////////////
// ACCESSORS
//////////////////////////////////////////////////////////////////////////

FolderPtr Project::getRoot() const
{
    return mRoot;
}

PropertiesPtr Project::getProperties() const
{
    return mProperties;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Project::serialize(Archive & ar, const unsigned int version)
{
    ar & mRoot;
    ar & mProperties;
}
template void Project::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Project::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace