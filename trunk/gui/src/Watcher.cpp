#include "Watcher.h"

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
    return mWatcher ? mWatcher->GetWatchedPathsCount() : 0;
}

//////////////////////////////////////////////////////////////////////////
// EVENT HANDLING
//////////////////////////////////////////////////////////////////////////

void Watcher::onChange(wxFileSystemWatcherEvent& event)
{
    VAR_INFO(event.ToString());
    VAR_INFO(GetFSWEventChangeTypeName(event.GetChangeType()))(event.GetPath())(event.GetNewPath());
    event.Skip();

    boost::optional<wxString> autoFolderPath = boost::none;// = event.GetPath().GetFullPath();
    boost::optional<wxString> filePath = boost::none; // For files that are not part of an autofolder tree

    switch (event.GetChangeType())
    {
    case wxFSW_EVENT_RENAME:
        break;
    case wxFSW_EVENT_CREATE:
        // Events can be ignored: A Modify event (of the parent folder) will occur afterwards.
        break;
    case wxFSW_EVENT_DELETE:
        // Events can not be ignored: For deletion of a (sub)folder only a delete event is sent.
    case wxFSW_EVENT_MODIFY:
        // Modify is particularly required for adding avi files to auto folders.
        // - First, a create event is given (at that point the file is not filled with valid avi data yet, so open file may fail).
        // - Second, modify events are given while the file contents is updated.
        {
            wxFileName dir(event.GetPath());
            dir.SetFullName("");
            wxString fp = dir.GetFullPath();
            model::NodePtrs nodes = model::Project::get().getRoot()->findPath(dir.GetFullPath());
            BOOST_FOREACH( model::NodePtr node, nodes )
            {
                model::AutoFolderPtr autoFolder = boost::static_pointer_cast<model::AutoFolder>(node);
                if (autoFolder)
                {
                    autoFolder->update();
                }
                else
                {
                    // Todo for files not in an autofolder tree also an update is required!
                }
            }
            break;
        }
    case wxFSW_EVENT_ACCESS:
        break;
    case wxFSW_EVENT_WARNING:
        break;
    case wxFSW_EVENT_ERROR:
        {
        // A watched root was removed
        model::NodePtrs nodes = model::Project::get().getRoot()->findPath(event.GetPath().GetFullPath());
        VAR_ERROR(nodes);
        BOOST_FOREACH( model::NodePtr node, nodes )
        {
            model::NodePtr parent = node->getParent();
            ASSERT(parent);
            parent->removeChild(node);
        }
        stop(); // Restart fs watcher
        start();
        return; // Don't update any folders
        }
    case wxFSW_EVENT_ATTRIB:
        break;
    }

    // todo use node->mustBeWatched(path);

    //model::NodePtrs nodes = model::Project::get().getRoot()->findPath(watchedPath);
    //VAR_ERROR(nodes);

    //BOOST_FOREACH( model::NodePtr node, nodes )
    //{
    //    if (node->isA<model::AutoFolder>())
    //    {
    //        boost::static_pointer_cast<model::AutoFolder>(node)->update(false);
    //    }
    //    else if (node->getParent() && node->getParent()->isA<model::AutoFolder>())
    //    {
    //        boost::static_pointer_cast<model::AutoFolder>(node->getParent())->update(false);
    //    }
    //}
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

    wxString toBeWatched = (node->isA<model::AutoFolder>()) ? pathOnDisk->getPath().GetFullPath() : pathOnDisk->getPath().GetPath();

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
        if (toBeWatched.StartsWith(alreadyWatchedPath))
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
        if (kv.first.StartsWith(toBeWatched))
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
    mWatcher->AddTree(wxFileName(toBeWatched,""));
}

void Watcher::unwatch( model::NodePtr node )
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    if (!requiresWatch) { return; }

    boost::optional<wxString> toBeRemoved = boost::none;
    BOOST_FOREACH( MapFolderToNodes::value_type kv, mWatches )
    {
        if (kv.second.count(node) == 1)
        {
            kv.second.erase(node);
            if (kv.second.empty())
            {
                toBeRemoved = boost::optional<wxString>(kv.first);
                // Last node for which this folder was watched. Folder should no longer be watched.
                mWatcher->RemoveTree(wxFileName(*toBeRemoved,""));
            }
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

    // todo:
    // - use wxFSW_EVENT_WARNING see http://trac.wxwidgets.org/ticket/12847
    // - use addtree for adding auto folders (recursively)
    // - use fix for http://trac.wxwidgets.org/ticket/13294 (use the event error to restart)

    return "INVALID_TYPE";
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Watcher::serialize(Archive & ar, const unsigned int version)
{
    //if (Archive::is_loading::value)
    //{
    //    start();
    //}
}
template void Watcher::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Watcher::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace