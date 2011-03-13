#ifndef MODEL_FSWATCHER_H
#define MODEL_FSWATCHER_H

#include <wx/fswatcher.h>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include "ModelPtr.h"

inline bool operator<(wxFileName l, const wxFileName& r) { return l.GetFullPath() > r.GetFullPath(); }

namespace model {

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

    void watchFolder(AutoFolderPtr folder);
    void watchFile(FilePtr file);

    void unwatchFolder(AutoFolderPtr folder);
    void unwatchFile(FilePtr file);

protected:

    //////////////////////////////////////////////////////////////////////////
    // EVENT HANDLING
    //////////////////////////////////////////////////////////////////////////

    void OnChange(int changeType, const wxFileName& path, const wxFileName& newPath);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    typedef std::list<AutoFolderPtr> FolderList;
    typedef std::list<FilePtr> FileList;
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

#endif // MODEL_FSWATCHER_H
