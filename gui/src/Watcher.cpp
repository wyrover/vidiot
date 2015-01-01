// Copyright 2013-2015 Eric Raijmakers.
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

#include "Watcher.h"

#include "AutoFolder.h"
#include "File.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "NodeEvent.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilVector.h"
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

    gui::Window::get().Bind(model::EVENT_ADD_NODE,     &Watcher::onProjectAssetAdded,    this);
    gui::Window::get().Bind(model::EVENT_ADD_NODES,    &Watcher::onProjectAssetsAdded,   this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &Watcher::onProjectAssetRemoved,  this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,  &Watcher::onProjectAssetRenamed,  this);

    start();
}

Watcher::~Watcher()
{
    VAR_DEBUG(this);

    gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &Watcher::onProjectAssetAdded,    this);
    gui::Window::get().Unbind(model::EVENT_ADD_NODES,      &Watcher::onProjectAssetsAdded,   this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &Watcher::onProjectAssetRemoved,  this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,    &Watcher::onProjectAssetRenamed,  this);

    mWatches.clear();
    stop();
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

int Watcher::getWatchedPathsCount() const
{
    return mWatches.size();
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
            wxFileName changedPath = event.GetPath();
            model::NodePtrs nodes = model::Project::get().getRoot()->findPath(changedPath.GetFullPath());
            if (!nodes.empty())
            {
                // Existing file: update
                for ( model::NodePtr node : nodes )
                {
                    node->check();
                }
            }
            else
            {
                changedPath.SetFullName(""); // Take the folder (this is probably a file that has just been created)
                model::NodePtrs nodes = model::Project::get().getRoot()->findPath(changedPath.GetFullPath());
                for ( model::NodePtr node : nodes )
                {
                    node->check();
                }
            }
            break;
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

void Watcher::onProjectAssetAdded(model::EventAddNode &event)
{
    watch( event.getValue().getChild() );
    event.Skip();
}

void Watcher::onProjectAssetsAdded(model::EventAddNodes &event)
{
    for ( model::NodePtr node : event.getValue().getChildren() )
    {
        watch( node );
    }
    event.Skip();
}

void Watcher::onProjectAssetRemoved(model::EventRemoveNode &event)
{
    unwatch( event.getValue().getChild() );
    event.Skip();
}

void Watcher::onProjectAssetsRemoved(model::EventRemoveNodes &event)
{
    for ( model::NodePtr node : event.getValue().getChildren() )
    {
        unwatch( node );
    }
    event.Skip();
}

void Watcher::onProjectAssetRenamed(model::EventRenameNode &event)
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// ADD/REMOVE
//////////////////////////////////////////////////////////////////////////

boost::optional<wxString> requiredWatchPath(const model::NodePtr& node)
{
    if (node->getParent() && node->getParent()->isA<model::AutoFolder>()) { return boost::none; } // The parent autofolder (in the project view) is already watched

    model::IPathPtr pathOnDisk = boost::dynamic_pointer_cast<model::IPath>(node);
    if (!pathOnDisk) { return boost::none; }// The node does not correspond to a file on disk.

    ASSERT(node->isA<model::AutoFolder>() || node->isA<model::File>());

    wxFileName path = pathOnDisk->getPath();
    VAR_DEBUG(path);
    if (!node->isA<model::AutoFolder>())
    {
        path.SetFullName(""); // Remove filename and use the folder
    }

    return boost::optional<wxString>(util::path::toPath(path));
}

void Watcher::watch(const model::NodePtr& node)
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    if (!requiresWatch)
    {
        // The node itselves does not need watching, but maybe it's children do.
        // Can happen for a (non auto) folder containing AutoFolders/Files
        for ( model::NodePtr child : node->getChildren() )
        {
            watch(child);
        }
        return;
    }
    wxString toBeWatched = *requiresWatch;

    for ( MapFolderToNodes::value_type kv : mWatches )
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
    wxStrings watchesToBeRemoved;
    for ( MapFolderToNodes::value_type kv : mWatches )
    {
        if (util::path::isParentOf(toBeWatched, kv.first ))
        {
            // This is a child dir watch. All nodes in that watch are now covered by the parent folder watch.
            watchesToBeRemoved.push_back(kv.first);
            nodesToBeTransferred.insert(kv.second.begin(),kv.second.end());
        }
    }

    NodeSet newset;
    newset.insert(node);
    newset.insert(nodesToBeTransferred.begin(),nodesToBeTransferred.end());
    mWatches[toBeWatched] = newset;
    for ( wxString obsoleteWatch : watchesToBeRemoved )
    {
        mWatches.erase(obsoleteWatch);
        mWatcher->RemoveTree(wxFileName(obsoleteWatch,""));
    }

    // Start watching the new folder
    VAR_DEBUG(mWatches);
    if (!mWatcher)
    {
        start();
    }
    bool ok = mWatcher->AddTree(wxFileName(toBeWatched + "/",""));
    if (!ok)
    {
        VAR_WARNING(toBeWatched);
        stop();
        start();
    }
    ASSERT_MORE_THAN_EQUALS(mWatcher->GetWatchedPathsCount(), static_cast<int>(mWatches.size())); // Note that sometimes both 'folder' and 'folder/' are added.
    VAR_DEBUG(*this);
}

void Watcher::unwatch(const model::NodePtr& node)
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    if (!requiresWatch)
    {
        // The node itselves does not need watching, but maybe it's children do.
        // Can happen for a (non auto) folder containing AutoFolders/Files
        for ( model::NodePtr child : node->getChildren() )
        {
            unwatch(child);
        }
        return;
    }

    wxString obsoleteWatch = *requiresWatch;

    // Sometimes, the given folder is not watched, because one of its parents is
    if (mWatches.count(obsoleteWatch) == 0) { return; }

    ASSERT_EQUALS(mWatches.count(obsoleteWatch),1);
    ASSERT_EQUALS(mWatches[obsoleteWatch].count(node),1);

    mWatches[obsoleteWatch].erase(node);

    if (mWatches[obsoleteWatch].empty())
    {
        mWatches.erase(obsoleteWatch);
        if (mWatcher)
        {
            bool ok = mWatcher->RemoveTree(wxFileName(obsoleteWatch,""));
            if (!ok)
            {
                VAR_WARNING(obsoleteWatch);
                stop();
                start();
            }
            else
            {
                ASSERT_MORE_THAN_EQUALS(mWatcher->GetWatchedPathsCount(), static_cast<int>(mWatches.size())); // Note that sometimes both 'folder' and 'folder/' are added.
                if (mWatches.empty())
                {
                    stop();
                }
            }
        }
    }
    VAR_DEBUG(*this);
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
    VAR_DEBUG(*this);
    for ( MapFolderToNodes::value_type kv : mWatches )
    {
        mWatcher->AddTree(kv.first);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Watcher& obj)
{
    os << &obj << '|' << obj.mWatches;
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
