#include "Watcher.h"

#include "AutoFolder.h"
#include "File.h"
#include "ProjectEvent.h"
#include "NodeEvent.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Watcher::Watcher()
    :   mWatcher(0)
    ,   mWatches()
{
    VAR_DEBUG(this);
    gui::Window::get().Bind(model::EVENT_OPEN_PROJECT,     &Watcher::onOpenProject,           this);
    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,    &Watcher::onCloseProject,          this);

    start();
}

Watcher::~Watcher()
{
    VAR_DEBUG(this);

    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,   &Watcher::onOpenProject,            this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,  &Watcher::onCloseProject,           this);

    stop();
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

int Watcher::getWatchedPathsCount() const
{
    if (!mWatcher)
    {
        ASSERT_ZERO(mWatches.size());
        return 0;
    }

    wxArrayString paths;
    int nPath = mWatcher->GetWatchedPaths (&paths);
    for ( int i = 0; i < nPath; ++i ) { VAR_DEBUG(paths[i]); }
    return nPath;
}

//////////////////////////////////////////////////////////////////////////
// EVENT HANDLING
//////////////////////////////////////////////////////////////////////////

void Watcher::onChange(wxFileSystemWatcherEvent& event)
{
    VAR_INFO(event.ToString());
    VAR_INFO(GetFSWEventChangeTypeName(event.GetChangeType()))(event.GetPath())(event.GetNewPath());
    event.Skip();

    boost::optional<wxString> autoFolderPath = boost::none;
    boost::optional<wxString> filePath = boost::none;

    switch (event.GetChangeType())
    {
    case wxFSW_EVENT_RENAME:
        break;
    case wxFSW_EVENT_CREATE:
        // Events can be ignored: A Modify event (of the parent folder) will occur afterwards.
        break;
    case wxFSW_EVENT_WARNING:
        // Triggered by errors in the wxFileSystemWatcher caused by ReadDirectoryChangesW() buffer overflow
    case wxFSW_EVENT_ERROR:
        // Triggered if root watch is deleted from disk
    case wxFSW_EVENT_DELETE:
        // Events can not be ignored: For deletion of a (sub)folder only a delete event is sent.
    case wxFSW_EVENT_MODIFY:
        // Modify is particularly required for adding avi files to auto folders.
        // - First, a create event is given (at that point the file is not filled with valid avi data yet, so open file may fail).
        // - Second, modify events are given while the file contents is updated.
        {
            // Check which autofolders need updating
            wxFileName dir(event.GetPath());
            dir.SetFullName("");
            BOOST_FOREACH( model::NodePtr node,  model::Project::get().getRoot()->findPath(dir.GetFullPath()) )
            {
                if (node->isA<model::AutoFolder>())
                {
                    boost::dynamic_pointer_cast<model::AutoFolder>(node)->update();
                }
            }
            // Check if there are files that have been removed, but are not part of an auto folder hierarchy
            BOOST_FOREACH( model::NodePtr node, model::Project::get().getRoot()->findPath(event.GetPath().GetFullPath()) )
            {
                model::NodePtr parent = node->getParent();
                ASSERT(parent);
                if (parent->isA<model::AutoFolder>())
                {
                    // Already updated via autofolder indexing above.
                    continue;
                }
                parent->removeChild(node);
                model::IPathPtr path = boost::dynamic_pointer_cast<model::IPath>(node);
                ASSERT(path);
                gui::Dialog::get().getConfirmation(_("File removed"), _("The file ") + util::path::toName(path->getPath()) + _(" has been removed from disk. File is removed from project also."));
           }
            break; // todo add a method to IPath : such that these objects handle the rescanning themselves
        }
    case wxFSW_EVENT_ACCESS:
        break;
    case wxFSW_EVENT_ATTRIB:
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void Watcher::onOpenProject( model::EventOpenProject &event )
{
    gui::Window::get().Bind(model::EVENT_ADD_NODE,     &Watcher::onProjectAssetAdded,    this);
    gui::Window::get().Bind(model::EVENT_ADD_NODES,    &Watcher::onProjectAssetsAdded,   this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &Watcher::onProjectAssetRemoved,  this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,  &Watcher::onProjectAssetRenamed,  this);

    event.Skip();
}

void Watcher::onCloseProject( model::EventCloseProject &event )
{
    gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &Watcher::onProjectAssetAdded,    this);
    gui::Window::get().Unbind(model::EVENT_ADD_NODES,      &Watcher::onProjectAssetsAdded,   this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &Watcher::onProjectAssetRemoved,  this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,    &Watcher::onProjectAssetRenamed,  this);

    event.Skip();
}

void Watcher::onProjectAssetAdded( model::EventAddNode &event )
{
    watch( event.getValue().getChild() );
    event.Skip();
}

void Watcher::onProjectAssetsAdded( model::EventAddNodes &event )
{
    BOOST_FOREACH( model::NodePtr node, event.getValue().getChildren() )
    {
        watch( node );
    }
    event.Skip();
}

void Watcher::onProjectAssetRemoved( model::EventRemoveNode &event )
{
    unwatch( event.getValue().getChild() );
    event.Skip();
}

void Watcher::onProjectAssetsRemoved( model::EventRemoveNodes &event )
{
    BOOST_FOREACH( model::NodePtr node, event.getValue().getChildren() )
    {
        unwatch( node );
    }
    event.Skip();
}

void Watcher::onProjectAssetRenamed( model::EventRenameNode &event )
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// ADD/REMOVE
//////////////////////////////////////////////////////////////////////////

boost::optional<wxString> requiredWatchPath( model::NodePtr node )
{
    if (node->getParent() && node->getParent()->isA<model::AutoFolder>()) { return boost::none; } // The parent autofolder (in the project view) is already watched

    model::IPathPtr pathOnDisk = boost::dynamic_pointer_cast<model::IPath>(node);
    if (!pathOnDisk) { return boost::none; }// The node does not correspond to a file on disk.

    ASSERT(node->isA<model::AutoFolder>() || node->isA<model::File>());

    wxString toBeWatched = node->isA<model::AutoFolder>() ? pathOnDisk->getPath().GetFullPath() : pathOnDisk->getPath().GetPath();

    return boost::optional<wxString>(toBeWatched);
}

void Watcher::watch( model::NodePtr node )
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    if (!requiresWatch) { return; }
    wxString toBeWatched = *requiresWatch;

    BOOST_FOREACH( MapFolderToNodes::value_type kv, mWatches )
    {
        wxString alreadyWatchedPath = kv.first;
        bool isWatched = util::path::equals(alreadyWatchedPath, toBeWatched);
        bool isParentWatched = util::path::isParentOf(alreadyWatchedPath, toBeWatched );
        if (isWatched ||  isParentWatched)
        {
            mWatches[alreadyWatchedPath].insert(node); // This folder, or one of its parents is already watched. As long as this node is present, don't remove that watch
            return;
        }
    }

    ASSERT(mWatches.find(toBeWatched) == mWatches.end()); // The required watch path is not watched yet. Nor is one of its parents.

    // Check if a child folder of the new folder was already watched. If so, that watch is replaced by the new parent dir watch.
    NodeSet nodesToBeTransferred;
    std::list<wxString> watchesToBeRemoved;
    BOOST_FOREACH( MapFolderToNodes::value_type kv, mWatches )
    {
        if (util::path::isParentOf(toBeWatched, kv.first ))
        {
            // This is a child dir watch. All nodes in that watch are now covered by the parent folder watch.
            watchesToBeRemoved.push_back(kv.first);
            nodesToBeTransferred.insert(kv.second.begin(),kv.second.end());
        }
    }
    mWatches[toBeWatched] = boost::assign::list_of(node);
    mWatches[toBeWatched].insert(nodesToBeTransferred.begin(),nodesToBeTransferred.end());
    BOOST_FOREACH( wxString obsoleteWatch, watchesToBeRemoved )
    {
        mWatches.erase(obsoleteWatch);
        mWatcher->RemoveTree(wxFileName(obsoleteWatch,""));
    }

    // Start watching the new folder
    VAR_DEBUG(mWatches);
    bool ok = mWatcher->AddTree(wxFileName(toBeWatched,""));
    if (!ok)
    {
        VAR_WARNING(toBeWatched);
        stop();
        start();
    }
}

void Watcher::unwatch( model::NodePtr node )
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    if (!requiresWatch) { return; }

    wxString obsoleteWatch = *requiresWatch;

    // Sometimes, the given folder is not watched, because one of its parents is
    if (mWatches.count(obsoleteWatch) == 0) { return; }

    ASSERT_EQUALS(mWatches.count(obsoleteWatch),1);
    ASSERT_EQUALS(mWatches[obsoleteWatch].count(node),1);

    mWatches[obsoleteWatch].erase(node);

    if (mWatches[obsoleteWatch].empty())
    {
        mWatches.erase(obsoleteWatch);
        bool ok = mWatcher->RemoveTree(wxFileName(obsoleteWatch,""));
        if (!ok)
        {
            VAR_WARNING(obsoleteWatch);
            stop();
            start();
        }
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
    ASSERT(!mWatcher);
    mWatcher = new wxFileSystemWatcher();
    mWatcher->Bind(wxEVT_FSWATCHER, &Watcher::onChange, this);
    BOOST_FOREACH( MapFolderToNodes::value_type kv, mWatches )
    {
        VAR_DEBUG(kv.first);
        mWatcher->AddTree(kv.first);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Watcher& obj )
{
    os << &obj;
    return os;
}

// static
wxString Watcher::GetFSWEventChangeTypeName(int changeType)
{
    switch (changeType)
    {
    case wxFSW_EVENT_CREATE:  return "CREATE";
    case wxFSW_EVENT_DELETE:  return "DELETE";
    case wxFSW_EVENT_RENAME:  return "RENAME";
    case wxFSW_EVENT_MODIFY:  return "MODIFY";
    case wxFSW_EVENT_ACCESS:  return "ACCESS";
    case wxFSW_EVENT_WARNING: return "WARNING";
    case wxFSW_EVENT_ERROR:   return "ERROR";
    case wxFSW_EVENT_ATTRIB:  return "ATTRIB";
    }

    return "INVALID_TYPE";
}

} //namespace