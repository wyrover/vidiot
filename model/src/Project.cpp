#include "Project.h"

#include <wx/msgdlg.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "Folder.h"
#include "Main.h"
#include "Properties.h"
#include "UtilLog.h"

namespace model {

DEFINE_EVENT(EVENT_OPEN_PROJECT,    EventOpenProject,   model::Project*);
DEFINE_EVENT(EVENT_CLOSE_PROJECT,   EventCloseProject,  model::Project*);

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

    EventCloseProject closeEvent(this); // Do not 'inline' in the next line like gui::wxGetApp().ProcessEvent(EventCloseProject(this)); Doesn't compile in g++
    gui::wxGetApp().ProcessEvent(closeEvent);
    return wxDocument::OnCloseDocument();
}

bool Project::OnNewDocument()
{
    bool opened = wxDocument::OnNewDocument();
    if (opened)
    {
        gui::wxGetApp().QueueEvent(new EventOpenProject(this));
    }
    return opened;
}

bool Project::OnOpenDocument(const wxString& file)
{
    bool opened = wxDocument::OnOpenDocument(file);
    if (opened)
    {
        // This event is sent as late as possible. This ensures that no 'addChild' events
        // will be received by the widgets during loading from xml. Rationale: these
        // events are sent in a bottom-up - thus innermost child first - fashion, which
        // confuses the project view, since children are added before their parents.
        //wxGetApp().QueueEvent(new ProjectEventOpenProject(PROJECT_EVENT_OPEN_PROJECT,this));
    }
    return opened;
}

bool Project::OnSaveDocument(const wxString& file)
{
    // /todo         clear commands
    return wxDocument::OnSaveDocument(file);
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

void Project::OnChangeFilename(bool notifyViews)
{
    mRoot->setName(GetUserReadableName());
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
    /** \todo wait until worker is done, and ensure that no new work is generated */
    try
    {
        boost::archive::text_oarchive ar(ostream);
        ar & *this;
        ar & gui::wxGetApp();
    }
    catch (boost::archive::archive_exception* e)
    {
        wxMessageBox(e->what()); /** /todo bettter handling */
    }
    catch (std::exception* e)
    {
        wxMessageBox(e->what()); /** /todo bettter handling */
    }
    catch (...)
    {
        wxMessageBox("aiai"); /** /todo bettter handling */

    }
    return ostream;
}

std::istream& Project::LoadObject(std::istream& istream)
{
    try
    {
        boost::archive::text_iarchive ar(istream);
        ar & *this;
        ar & gui::wxGetApp();
        gui::wxGetApp().QueueEvent(new EventOpenProject(this)); /** @todo do not submit via app, but via individual nodes */
    }
    //catch (std::exception* e)
    //{
    //    wxMessageBox(e->what()); /** /todo bettter handling */
    //}
    catch (boost::archive::archive_exception* e)
    {
        wxMessageBox(e->what()); /** /todo bettter handling */
    }
    /** /todo postporc */
    return istream;
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
