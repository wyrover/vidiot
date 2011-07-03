#ifndef WATCHER_H
#define WATCHER_H

#include <boost/filesystem/path.hpp>
#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <map>
#include <wx/FSWatcher.h>

inline bool operator<(wxFileName l, const wxFileName& r) { return l.GetFullPath() < r.GetFullPath(); }

namespace model {
    class INode;
    typedef boost::shared_ptr<INode> NodePtr;
    typedef std::list<NodePtr> NodePtrs;
    class AutoFolder;
    typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;
    class File;
    typedef boost::shared_ptr<File> FilePtr;
    class EventOpenProject;
    class EventCloseProject;
    class EventAddAsset;
    class EventRemoveAsset;
    class EventRenameAsset;
}

namespace  gui {

class Watcher
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Watcher();
    virtual ~Watcher();

    /// Get the current project's watcher. Used to avoid having a dependency to
    /// the Project class throughout the code.
    static Watcher* current();

protected:

    //////////////////////////////////////////////////////////////////////////
    // EVENT HANDLING
    //////////////////////////////////////////////////////////////////////////

    void onChange(wxFileSystemWatcherEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    typedef std::map<wxFileName, model::NodePtrs> FileMap;
    FileMap mFileMap;


    // See wxWidgets Ticket #13294. To avoid this issue, Watcher is not
    // derived from wxFileSystemWatcher. Instead, mWatcher is going to be
    // destroyed and restarted again, upon each change. This ensures that
    // the overlapped io is aborted (PostEmptyStatus() in file watcher 
    // internals is called when destroying the watcher, which does the trick).
     wxFileSystemWatcher* mWatcher;

     bool mRestartRequired;

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpenProject( model::EventOpenProject &event );
    void onCloseProject( model::EventCloseProject &event );
    void onProjectAssetAdded( model::EventAddAsset &event );
    void onProjectAssetRemoved( model::EventRemoveAsset &event );
    void onProjectAssetRenamed( model::EventRenameAsset &event );

	//////////////////////////////////////////////////////////////////////////
	// ADD/REMOVE
	//////////////////////////////////////////////////////////////////////////

    void watch( model::NodePtr node, wxFileName path );
    void unwatch( model::NodePtr node, wxFileName path );

    void stop();
    void start();
    void restart();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Watcher& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

#endif // WATCHER_H
