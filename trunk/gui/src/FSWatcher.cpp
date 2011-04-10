#include "FSWatcher.h"

#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilLogStl.h"
#include "AutoFolder.h"
#include "File.h"
#include "UtilList.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static FSWatcher* sCurrent = 0;

FSWatcher::FSWatcher()
:	wxFileSystemWatcher()
,   mFolders()
,   mFiles()
{
    VAR_DEBUG(this);
    sCurrent = this;
    Bind(wxEVT_FSWATCHER, &FSWatcher::onChange, this);
}

FSWatcher::~FSWatcher()
{
    VAR_DEBUG(this);
    Unbind(wxEVT_FSWATCHER, &FSWatcher::onChange, this);
    mFolders.clear();
    mFiles.clear();
    sCurrent = 0;
}

FSWatcher* FSWatcher::current()
{
    return sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// ADD
//////////////////////////////////////////////////////////////////////////

void FSWatcher::watchFolder(model::AutoFolder* folder)
{
    VAR_DEBUG(folder->getFileName());
    FolderMap::iterator it = mFolders.find(folder->getFileName());
    if (it == mFolders.end())
    {
        // First folder for this path
        mFolders[folder->getFileName()] = boost::assign::list_of(folder);
        Add(folder->getFileName());
    }
    else
    {
        ASSERT(!UtilList<model::AutoFolder*>(it->second).hasElement(folder))(*it);
        it->second.push_back(folder);
    }
}

void FSWatcher::watchFile(model::File* file)
{
    VAR_DEBUG(file->getFileName());
    FileMap::iterator it = mFiles.find(file->getFileName());
    if (it == mFiles.end())
    {
        // First file for this path. Start watching.
        mFiles[file->getFileName()] = boost::assign::list_of(file);
        Add(file->getFileName());
    }
    else
    {
        ASSERT(!UtilList<model::File*>(it->second).hasElement(file))(*it);
        it->second.push_back(file);
    }
}

void FSWatcher::unwatchFolder(model::AutoFolder* folder)
{
    VAR_DEBUG(folder->getFileName());
    ASSERT(mFolders.find(folder->getFileName()) != mFolders.end());
    UtilList<model::AutoFolder*>(mFolders[folder->getFileName()]).removeElements(boost::assign::list_of(folder));
    if (mFolders[folder->getFileName()].empty())
    {
        // Last entry removed. Remove entire list and stop watching
        mFolders.erase(folder->getFileName());
        Remove(folder->getFileName());
    }
}

void FSWatcher::unwatchFile(model::File* file)
{
    VAR_DEBUG(file->getFileName());
    ASSERT(mFiles.find(file->getFileName()) != mFiles.end());
    UtilList<model::File*>(mFiles[file->getFileName()]).removeElements(boost::assign::list_of(file));
    if (mFiles[file->getFileName()].empty())
    {
        // Last entry removed. Remove entire list and stop watching
        mFiles.erase(file->getFileName());
        Remove(file->getFileName());
    }
}

//////////////////////////////////////////////////////////////////////////
// EVENT HANDLING
//////////////////////////////////////////////////////////////////////////

void FSWatcher::onChange(wxFileSystemWatcherEvent& event)
{
    VAR_INFO(event.GetChangeType())(event.GetPath())(event.GetNewPath());

    wxFileName changedfolder(event.GetPath().GetPath(),"");

    switch (event.GetChangeType())
    {
    case wxFSW_EVENT_CREATE:
        LOG_DEBUG << "CREATE"; 
        break;
    case wxFSW_EVENT_DELETE:
        LOG_DEBUG << "DELETE"; 
        break;
    case wxFSW_EVENT_RENAME: 
        LOG_DEBUG << "RENAME"; 
        FATAL("Do not rename files");
        break;
    case wxFSW_EVENT_MODIFY: 
        LOG_DEBUG << "MODIFY"; 
        break;
    case wxFSW_EVENT_ACCESS: 
        LOG_DEBUG << "ACCESS"; 
        return;
    default: FATAL("Unsupprted event type.");
    }

    if (changedfolder.IsDir())
    {
        ASSERT(mFolders.find(changedfolder) != mFolders.end());
        BOOST_FOREACH( model::AutoFolder* folder, mFolders.find(changedfolder)->second )
        {
            folder->update();
        }
    }
    else
    {
        //NOT: ASSERT(mFiles.find(event.GetPath()) != mFiles.end()); Since Folder wathcing also causes file events
        if (mFiles.find(event.GetPath()) != mFiles.end())
        {
            BOOST_FOREACH( model::File* file, mFiles.find(event.GetPath())->second )
            {
                //file->update();
            }

        }
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const FSWatcher& obj )
{
    os << &obj << '|' << obj.mFolders << '|' << obj.mFiles;
    return os;
}

static wxString GetFSWEventChangeTypeName(int changeType)
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


} //namespace
