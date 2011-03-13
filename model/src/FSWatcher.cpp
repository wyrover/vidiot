#include "FSWatcher.h"

#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilLogStl.h"
#include "AutoFolder.h"
#include "File.h"
#include "UtilList.h"

namespace model {

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
}

FSWatcher::~FSWatcher()
{
    VAR_DEBUG(this);
    ASSERT(mFolders.empty());
    ASSERT(mFiles.empty());
    sCurrent = 0;
}

FSWatcher* FSWatcher::current()
{
    return sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// ADD
//////////////////////////////////////////////////////////////////////////

void FSWatcher::watchFolder(AutoFolderPtr folder)
{
    FolderMap::iterator it = mFolders.find(folder->getFileName());
    if (it == mFolders.end())
    {
        // First folder for this path
        mFolders[folder->getFileName()] = boost::assign::list_of(folder);
        Add(folder->getFileName());
    }
    else
    {
        ASSERT(!UtilList<AutoFolderPtr>(it->second).hasElement(folder))(*it);
        it->second.push_back(folder);
    }
}

void FSWatcher::watchFile(FilePtr file)
{
    FileMap::iterator it = mFiles.find(file->getFileName());
    if (it == mFiles.end())
    {
        // First file for this path. Start watching.
        mFiles[file->getFileName()] = boost::assign::list_of(file);
        Add(file->getFileName());
    }
    else
    {
        ASSERT(!UtilList<FilePtr>(it->second).hasElement(file))(*it);
        it->second.push_back(file);
    }
}

void FSWatcher::unwatchFolder(AutoFolderPtr folder)
{
    ASSERT(mFolders.find(folder->getFileName()) != mFolders.end());
    UtilList<AutoFolderPtr>(mFolders[folder->getFileName()]).removeElements(boost::assign::list_of(folder));
    if (mFolders[folder->getFileName()].empty())
    {
        // Last entry removed. Remove entire list and stop watching
        mFolders.erase(folder->getFileName());
        Remove(folder->getFileName());
    }
}

void FSWatcher::unwatchFile(FilePtr file)
{
    ASSERT(mFiles.find(file->getFileName()) != mFiles.end());
    UtilList<FilePtr>(mFiles[file->getFileName()]).removeElements(boost::assign::list_of(file));
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

void FSWatcher::OnChange(int changeType, const wxFileName& path, const wxFileName& newPath)
{
    VAR_INFO(changeType)(path)(newPath);
    if (path.IsDir())
    {
        ASSERT(mFolders.find(path) != mFolders.end());
        BOOST_FOREACH( AutoFolderPtr folder, mFolders.find(path)->second )
        {
            folder->update();
        }
    }
    else
    {
        ASSERT(mFiles.find(path) != mFiles.end());
        BOOST_FOREACH( FilePtr file, mFiles.find(path)->second )
        {
            //file->update();
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

} //namespace
