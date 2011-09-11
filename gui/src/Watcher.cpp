#include "Watcher.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "AutoFolder.h"
#include "File.h"
#include "ProjectEvent.h"
#include "NodeEvent.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static Watcher* sCurrent = 0;

Watcher::Watcher()
    :   mFileMap()
    ,   mWatcher(0)
    ,   mRestartRequired(false)
{
    VAR_DEBUG(this);
    sCurrent = this;

    gui::Window::get().Bind(model::EVENT_OPEN_PROJECT,     &Watcher::onOpenProject,           this);
    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,    &Watcher::onCloseProject,          this);
}

Watcher::~Watcher()
{
    VAR_DEBUG(this);

    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,   &Watcher::onOpenProject,            this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,  &Watcher::onCloseProject,           this);

    stop();

    sCurrent = 0;
}

Watcher* Watcher::current()
{
    return sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// EVENT HANDLING
//////////////////////////////////////////////////////////////////////////

void Watcher::onChange(wxFileSystemWatcherEvent& event)
{
    // Get the parent dir.
    wxFileName file(event.GetPath());
    if (file.IsDir())
    {
        file.RemoveLastDir(); 
    }
    else
    {
        file.SetName("");
        file.ClearExt();
    }
    wxFileName folder( file.GetLongPath(), "" );

    VAR_INFO(folder)(GetFSWEventChangeTypeName(event.GetChangeType()))(event.GetPath())(event.GetNewPath());

    if (mFileMap.find(folder) != mFileMap.end())
    {
        BOOST_FOREACH( model::NodePtr node, mFileMap.find(folder)->second )
        {
            if (node->isA<model::AutoFolder>())
            {
                boost::static_pointer_cast<model::AutoFolder>(node)->update();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void Watcher::onOpenProject( model::EventOpenProject &event )
{
    gui::Window::get().Bind(model::EVENT_ADD_NODE,     &Watcher::onProjectAssetAdded,    this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &Watcher::onProjectAssetRemoved,  this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,  &Watcher::onProjectAssetRenamed,  this);

    event.Skip();
}

void Watcher::onCloseProject( model::EventCloseProject &event )
{
    gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &Watcher::onProjectAssetAdded,    this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &Watcher::onProjectAssetRemoved,  this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,    &Watcher::onProjectAssetRenamed,  this);
    
    event.Skip();
}

wxFileName getFileName( model::NodePtr node )
{
    if (node->isA<model::AutoFolder>())
    {
        return boost::static_pointer_cast<model::AutoFolder>(node)->getPath();
    }
    return wxFileName();
}

bool isWatchable( model::NodePtr node )
{
    // wxFileSystemWatcher does not support monitoring files yet.
    // Therefore model::File is not watched yet.
    return node->isA<model::AutoFolder>();
}

void Watcher::onProjectAssetAdded( model::EventAddNode &event )
{
    model::NodePtr node = event.getValue().getChild();
    watch( node, getFileName(node) );
    restart();
    event.Skip();
}

void Watcher::onProjectAssetRemoved( model::EventRemoveNode &event )
{
    model::NodePtr node = event.getValue().getChild();
    unwatch( node, getFileName(node) );
    restart();
    event.Skip();
}

void Watcher::onProjectAssetRenamed( model::EventRenameNode &event )
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// ADD/REMOVE
//////////////////////////////////////////////////////////////////////////

void Watcher::watch( model::NodePtr node, wxFileName path )
{
    if (isWatchable(node))
    {
        VAR_DEBUG(path);
        FileMap::iterator it = mFileMap.find(path);
        if (it == mFileMap.end())
        {
            // First file for this path
            mFileMap[path] = boost::assign::list_of(node);
            mRestartRequired = true;
        }
        else if (!UtilList<model::NodePtr>(it->second).hasElement(node))
        {
            it->second.push_back(node);
        }
        // else: This (name,file) combination is already watched
    }

    BOOST_FOREACH( model::NodePtr child, node->getChildren() )
    {
        watch(child, getFileName(child));
    }
}

void Watcher::unwatch( model::NodePtr node, wxFileName path )
{
    if (isWatchable(node))
    {
        VAR_DEBUG(path);
        ASSERT_MAP_CONTAINS(mFileMap,path);
        UtilList<model::NodePtr>(mFileMap[path]).removeElements(boost::assign::list_of(node));
        if (mFileMap[path].empty())
        {
            // Last entry removed. Remove entire list and stop watching
            mFileMap.erase(path);
            mRestartRequired = true;
        }
    }
    BOOST_FOREACH( model::NodePtr child, node->getChildren() )
    {
        unwatch(child, getFileName(child));
    }
}

void Watcher::stop()
{
    if (mWatcher)
    {
        mWatcher->Unbind(wxEVT_FSWATCHER, &Watcher::onChange, this);
        delete mWatcher;
        mWatcher = 0;
    }
}

void Watcher::start()
{
    mWatcher = new wxFileSystemWatcher();
    mWatcher->Bind(wxEVT_FSWATCHER, &Watcher::onChange, this);
    BOOST_FOREACH( FileMap::value_type nameAndNode, mFileMap )
    {
        wxFileName fn = nameAndNode.first;
        mWatcher->Add(fn);
    }
}

void Watcher::restart()
{
    if (mRestartRequired)
    {
        stop();
        start();
        mRestartRequired = false;
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Watcher& obj )
{
    os << &obj << '|' << obj.mFileMap;
    return os;
}

// static 
wxString Watcher::GetFSWEventChangeTypeName(int changeType)
{
    switch (changeType)
    {
    case wxFSW_EVENT_CREATE:
        return "CREATE";
    case wxFSW_EVENT_DELETE:
        return "DELETE";
    case wxFSW_EVENT_RENAME:
        return "RENAME";
    case wxFSW_EVENT_MODIFY:
        return "MODIFY";
    case wxFSW_EVENT_ACCESS:
        return "ACCESS";
    }

    return "INVALID_TYPE";
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Watcher::serialize(Archive & ar, const unsigned int version)
{
    if (Archive::is_loading::value)
    {
        start();
    }
}
template void Watcher::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Watcher::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
