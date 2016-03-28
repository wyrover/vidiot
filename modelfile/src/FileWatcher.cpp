// Copyright 2013-2016 Eric Raijmakers.
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

#include "FileWatcher.h"

#include "AutoFolder.h"
#include "File.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "NodeEvent.h"
#include "UtilPath.h"
#include "UtilVector.h"
#include "Window.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FileWatcher::FileWatcher()
{
    VAR_DEBUG(this);

    gui::Window::get().Bind(model::EVENT_ADD_NODE,     &FileWatcher::onProjectAssetAdded,    this);
    gui::Window::get().Bind(model::EVENT_ADD_NODES,    &FileWatcher::onProjectAssetsAdded,   this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &FileWatcher::onProjectAssetRemoved,  this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,  &FileWatcher::onProjectAssetRenamed,  this);

    start();
    watchAll(Project::get().getRoot());
}

FileWatcher::~FileWatcher()
{
    VAR_DEBUG(this);

    gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &FileWatcher::onProjectAssetAdded,    this);
    gui::Window::get().Unbind(model::EVENT_ADD_NODES,      &FileWatcher::onProjectAssetsAdded,   this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &FileWatcher::onProjectAssetRemoved,  this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,    &FileWatcher::onProjectAssetRenamed,  this);

    mWatches.clear();
    stop();
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

int FileWatcher::getWatchedPathsCount() const
{
    return mWatches.size();
}

//////////////////////////////////////////////////////////////////////////
// EVENT HANDLING
//////////////////////////////////////////////////////////////////////////

void FileWatcher::onChange(wxFileSystemWatcherEvent& event)
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
        // Events can not be ignored: A Modify event (of the parent folder) will not occur always afterwards.
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

void FileWatcher::onProjectAssetAdded(model::EventAddNode &event)
{
    watch( event.getValue().getChild() );
    event.Skip();
}

void FileWatcher::onProjectAssetsAdded(model::EventAddNodes &event)
{
    for ( model::NodePtr node : event.getValue().getChildren() )
    {
        watch( node );
    }
    event.Skip();
}

void FileWatcher::onProjectAssetRemoved(model::EventRemoveNode &event)
{
    unwatch( event.getValue().getChild() );
    event.Skip();
}

void FileWatcher::onProjectAssetsRemoved(model::EventRemoveNodes &event)
{
    for ( model::NodePtr node : event.getValue().getChildren() )
    {
        unwatch( node );
    }
    event.Skip();
}

void FileWatcher::onProjectAssetRenamed(model::EventRenameNode &event)
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

    wxString result(util::path::toPath(path));
    ASSERT(!result.IsSameAs(""));
    ASSERT(!wxEndsWithPathSeparator(result));
    return boost::optional<wxString>(result);
}

void FileWatcher::watchAll(const model::NodePtr& node)
{
    watch(node);
    for (NodePtr child : node->getChildren())
    {
        watchAll(child);
    }
}

void FileWatcher::watch(const model::NodePtr& node)
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    VAR_DEBUG(requiresWatch);
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
        if (isWatched || isParentWatched)
        {
            mWatches[alreadyWatchedPath].insert(node); // This folder, or one of its parents is already watched. As long as this node is present, don't remove that watch
            return;
        }
    }

    ASSERT(mWatches.find(toBeWatched) == mWatches.end()); // The required watch path is not watched yet. Nor is one of its parents.
    mWatches[toBeWatched] = { node }; // Watch new path
    // Original code used mWatcher->Remove/RemoveTree, but that caused too much errors.
    // (Quite often crashes somewhere in handling - late - notifications from the file systems,
    // both on Linux and Windows).
    stop();
    start();
    VAR_DEBUG(*this);
}

void FileWatcher::unwatch(const model::NodePtr& node)
{
    boost::optional<wxString> requiresWatch = requiredWatchPath(node);
    VAR_DEBUG(requiresWatch);
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
    ASSERT(!obsoleteWatch.IsSameAs(""));

    // Sometimes, the given folder is not watched, because one of its parents is, or if adding the watch failed.
    if (mWatches.count(obsoleteWatch) == 0) { return; }

    ASSERT_EQUALS(mWatches.count(obsoleteWatch),1);
    ASSERT_EQUALS(mWatches[obsoleteWatch].count(node),1);

    mWatches[obsoleteWatch].erase(node);
    if (mWatches[obsoleteWatch].empty())
    {
        mWatches.erase(obsoleteWatch);
    }
    // Original code used mWatcher->Remove/RemoveTree, but that caused too much errors.
    // (Quite often crashes somewhere in handling - late - notifications from the file systems,
    // both on Linux and Windows).
    stop();
    start();
    VAR_DEBUG(*this);
}

void FileWatcher::stop()
{
    if (mWatcher)
    {
        mWatcher->Unbind(wxEVT_FSWATCHER, &FileWatcher::onChange, this);
        delete mWatcher;
        mWatcher = 0;
    }
}

void FileWatcher::start()
{
    ASSERT(!mWatcher);
    mWatcher = new wxFileSystemWatcher();
    mWatcher->Bind(wxEVT_FSWATCHER, &FileWatcher::onChange, this);
    VAR_DEBUG(mWatches)(*this);
    for ( MapFolderToNodes::value_type kv : mWatches )
    {
        mWatcher->Add(wxFileName(kv.first,""));
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const FileWatcher& obj)
{
    os << &obj << '|' << obj.mWatches;
    return os;
}

// static
wxString FileWatcher::GetFSWEventChangeTypeName(int changeType)
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
