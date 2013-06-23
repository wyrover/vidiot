#include "Project.h"

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

Project::Project()
:   wxDocument()
// Do no initialize members with actual data here.
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
        ar & IView::getView();
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
        ar & IView::getView();
        IView::getView().ProcessModelEvent(EventOpenProject(this));
        mRoot->check();
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
        gui::Dialog::get().getConfirmation(_("Open Failed"),_("Could not open: " + file));
        return false;
    }
    LoadObject(store);
    if ( !store )
    {
        gui::Dialog::get().getConfirmation(_("Open Failed"),_("Could not read the contents of: " + file));
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// CHANGE COMMANDS
//////////////////////////////////////////////////////////////////////////

void Project::Submit(wxCommand* c)
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
    // Since the properties can be used by other objects, they must be read first.
    // An example is the framerate, which is used by 'Convert' which, in turn, is used in openFile() to determine the length of a stream in the file.
    ar & mProperties;
    ar & mRoot;
}
template void Project::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Project::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace