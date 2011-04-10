#ifndef FSWATCHER_H
#define FSWATCHER_H

#include <wx/fswatcher.h>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

inline bool operator<(wxFileName l, const wxFileName& r) { return l.GetFullPath() > r.GetFullPath(); }

namespace model {
    class AutoFolder;
    class File;
}

namespace  gui {

class FSWatcher
    :   public wxFileSystemWatcher
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FSWatcher();
    virtual ~FSWatcher();

    /// Get the current project's watcher. Used to avoid having a dependency to
    /// the Project class throughout the code.
    static FSWatcher* current();

	//////////////////////////////////////////////////////////////////////////
	// ADD/REMOVE
	//////////////////////////////////////////////////////////////////////////

    void watchFolder(model::AutoFolder* folder);
    void watchFile(model::File* file);

    void unwatchFolder(model::AutoFolder* folder);
    void unwatchFile(model::File* file);

protected:

    //////////////////////////////////////////////////////////////////////////
    // EVENT HANDLING
    //////////////////////////////////////////////////////////////////////////

    void onChange(wxFileSystemWatcherEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    typedef std::list<model::AutoFolder*> FolderList;
    typedef std::list<model::File*> FileList;
    typedef std::map<wxFileName, FolderList> FolderMap;
    typedef std::map<wxFileName, FileList> FileMap;

    FolderMap mFolders;
    FileMap mFiles;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const FSWatcher& obj );

};

} // namespace

#endif // FSWATCHER_H
