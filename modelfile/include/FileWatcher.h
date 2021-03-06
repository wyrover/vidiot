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

#pragma once

inline bool operator<(wxFileName l, const wxFileName& r) { return l.GetLongPath() < r.GetLongPath(); }

namespace model {
    class EventAddNode;
    class EventAddNodes;
    class EventRemoveNode;
    class EventRemoveNodes;
    class EventRenameNode;

/// For each node in the project view that is a file/dir on disk, the parent folder
/// must be watched for changes (to detect that the file/dir is removed).
class FileWatcher
    :   public SingleInstance<FileWatcher>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FileWatcher();
    virtual ~FileWatcher();

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    int getWatchedPathsCount() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // EVENT HANDLING
    //////////////////////////////////////////////////////////////////////////

    void onChange(wxFileSystemWatcherEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    typedef std::set< model::NodePtr > NodeSet;
    typedef std::map<wxString, NodeSet> MapFolderToNodes;
    MapFolderToNodes mWatches;

    // See wxWidgets Ticket #13294. To avoid this issue, Watcher is not
    // derived from wxFileSystemWatcher. Instead, mWatcher is going to be
    // destroyed and restarted again, upon each change. This ensures that
    // the overlapped io is aborted (PostEmptyStatus() in file watcher
    // internals is called when destroying the watcher, which does the trick).
     wxFileSystemWatcher* mWatcher = nullptr;

     bool mRestartRequired = false;

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onProjectAssetAdded(model::EventAddNode &event);
    void onProjectAssetsAdded(model::EventAddNodes &event);
    void onProjectAssetRemoved(model::EventRemoveNode &event);
    void onProjectAssetsRemoved(model::EventRemoveNodes &event);
    void onProjectAssetRenamed(model::EventRenameNode &event);

    //////////////////////////////////////////////////////////////////////////
    // ADD/REMOVE
    //////////////////////////////////////////////////////////////////////////

    void watchAll(const model::NodePtr& node );

    void watch(const model::NodePtr& node );
    void unwatch(const model::NodePtr& node );

    /// \return true if the given path or one of its parents is watched
    bool isWatched(const wxString& path) const;
    std::vector<wxString> redundantChildWatches(const wxString& path) const;
    bool parentFolderIsWatched(const wxString& path) const;

    boost::optional<wxString> getPathToBeWatched(const model::NodePtr& node) const;

    void stop();
    void start();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    static wxString GetFSWEventChangeTypeName(int changeType);
    friend std::ostream& operator<<(std::ostream& os, const FileWatcher& obj);
};

} // namespace
