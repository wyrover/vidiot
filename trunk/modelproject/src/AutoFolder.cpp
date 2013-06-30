#include "AutoFolder.h"

#include "Dialog.h"
#include "File.h"
#include "StatusBar.h"
#include "UtilLog.h"
#include "UtilList.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "Worker.h"
#include "WorkEvent.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// WORK OBJECT FOR INDEXING A FOLDER
//////////////////////////////////////////////////////////////////////////

struct IndexAutoFolderWork
    : public worker::Work
{
    // Here, all access to folder must be done, not in the worker thread.
    // Rationale: all access to model objects must be done in the main thread!
    explicit IndexAutoFolderWork(AutoFolderPtr folder)
        : worker::Work(boost::bind(&IndexAutoFolderWork::indexFiles,this))
        , mFolder(folder)
        , mPath(folder->getPath())
    {
        ASSERT(mPath.IsDir() && mPath.IsAbsolute())(mPath);
        BOOST_FOREACH( NodePtr child, mFolder->getChildren() )
        {
            mRemove.push_back(child->getName());
        }
    }

    void indexFiles()
    {
        setThreadName("IndexFiles");
        wxDir dir( mPath.GetLongPath() );
        mDirExists = dir.IsOpened();

        VAR_DEBUG(mPath.GetLongPath())(mDirExists);

        if (mDirExists)
        {
            wxString pathname =
                mPath.GetDirCount() >= 1 ? mPath.GetDirs().Last() + "/" :
                mPath.HasVolume() ? mPath.GetVolume() :
                "";
            showProgressText(_("Updating '") + pathname + "'");

            ASSERT(dir.IsOpened());
            wxString nodename;

            wxArrayString allfiles;
            size_t count = wxDir::GetAllFiles(mPath.GetLongPath(), &allfiles, wxEmptyString, wxDIR_FILES);
            showProgressBar(count);
            int progress = 0;

            // Find all subfolders
            for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_DIRS); cont; cont = dir.GetNext(&nodename))
            {
                if (isAborted()) { return; }
                if (UtilList<wxString>(mRemove).hasElement(nodename)) // Existing element. Do not remove
                {
                    UtilList<wxString>(mRemove).removeElements(boost::assign::list_of(nodename));
                }
                else // New element. Add
                {
                    wxFileName filename(mPath.GetLongPath(), "");
                    filename.AppendDir(nodename);
                    ASSERT(filename.IsDir());
                    AutoFolderPtr folder = boost::make_shared<AutoFolder>(filename);
                    mAdd.push_back(folder);
                }
                showProgress(++progress);
            }

            // Find all files
            for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_FILES); cont; cont = dir.GetNext(&nodename))
            {
                if (isAborted()) { return; }
                if (UtilList<wxString>(mRemove).hasElement(nodename)) // Existing element. Do not remove
                {
                    UtilList<wxString>(mRemove).removeElements(boost::assign::list_of(nodename));
                }
                else // New element. Add
                {
                    wxFileName filename(mPath.GetLongPath(), nodename);
                    showProgressText(_("Updating '") + pathname + nodename + "'");
                    model::FilePtr file = boost::make_shared<model::File>(filename);
                    if (file->canBeOpened())
                    {
                        mAdd.push_back(file);
                    }
                }
                showProgress(++progress);
            }
        }
        else
        {
            // This is ran in a separate thread.
            // - Maybe the folder has already been removed from disk, and the autofolder node has already been removed.
            // - Maybe the update is caused by deleting an entire folder hierarchy
            // Note that mRemove already contains all current nodes, these can be removed.
        }
    }

    AutoFolderPtr mFolder;               ///< Folder to be indexed
    bool mDirExists;                     ///< true if the dir could be opened, false if not (then, assume that it was removed)
    wxFileName mPath;                    ///< Path to the folder to be indexed
    std::list<wxString> mCurrentEntries; ///< All entries known when the work was scheduled
    NodePtrs mAdd;                       ///< When done, holds the list of nodes that must be added
    std::list<wxString> mRemove;         ///< All entries to be removed when indexing is done
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AutoFolder::AutoFolder()
    :   Folder()
    ,   mPath()
    ,   mLastModified(0)
    ,   mCurrentUpdate()
    ,   mUpdateAgain(false)
{
    VAR_DEBUG(this);
}

AutoFolder::AutoFolder(wxFileName path)
    :   Folder(util::path::toName(path))
    ,   mPath(util::path::normalize(path))
    ,   mLastModified(mPath.GetModificationTime().GetTicks())
    ,   mCurrentUpdate()
    ,   mUpdateAgain(false)
{
    ASSERT(path.IsDir())(path);
    VAR_DEBUG(this)(path);
}

AutoFolder::~AutoFolder()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// INODE
//////////////////////////////////////////////////////////////////////////

NodePtrs AutoFolder::findPath(wxString path)
{
    NodePtrs result = Node::findPath(path); // Traverse the tree
    if (util::path::equals(mPath,path))
    {
        result.push_back(shared_from_this());
    }
    return result;
}

bool AutoFolder::mustBeWatched(wxString path)
{
    if (util::path::isParentOf(path,mPath))
    {
        // Yes, is parent folder of this folder
        return true;
    }
    return Node::mustBeWatched(path); // Maybe for any of the children?
}

void AutoFolder::check()
{
    ASSERT(wxThread::IsMain());

    if (getParent() && getParent()->isA<AutoFolder>())
    {
        // Update parent also. Required for scenarios in which entire folder structures are removed. By updating
        // 'from the top' nothing is missed.
        boost::dynamic_pointer_cast<AutoFolder>(getParent())->check();
    }
    if (mCurrentUpdate)
    {
        mUpdateAgain = true; // When mCurrentUpdate is finished, another will be scheduled.
        VAR_DEBUG(mPath)(mUpdateAgain);
    }
    else
    {
        VAR_DEBUG(mPath);
        mCurrentUpdate = boost::make_shared<IndexAutoFolderWork>(boost::dynamic_pointer_cast<AutoFolder>(shared_from_this()));
        mCurrentUpdate->Bind(worker::EVENT_WORK_DONE, &AutoFolder::onWorkDone, this); // No unbind: work object is destroyed when done
        worker::Worker::get().schedule(mCurrentUpdate);
    }
    return;
}

//////////////////////////////////////////////////////////////////////////
// IPATH
//////////////////////////////////////////////////////////////////////////

wxFileName AutoFolder::getPath() const
{
    return mPath;
}

void AutoFolder::onWorkDone(worker::WorkDoneEvent& event)
{
    ASSERT(wxThread::IsMain());
    boost::shared_ptr<IndexAutoFolderWork> work = boost::dynamic_pointer_cast<IndexAutoFolderWork>(event.getValue());
    ASSERT_EQUALS(mCurrentUpdate,work)(mCurrentUpdate)(work);
    ASSERT(work);
    mCurrentUpdate.reset();
    if (!work->mAdd.empty())
    {
        addChildren(work->mAdd); // Add all at once, for better performance (less UI updates)
        BOOST_FOREACH( NodePtr node, work->mAdd )
        {
            AutoFolderPtr autoFolder = boost::dynamic_pointer_cast<AutoFolder>(node);
            if (autoFolder)
            {
                autoFolder->check();
            }
        }
    }

    if (!work->mRemove.empty())
    {
        model::NodePtrs nodes;
        BOOST_FOREACH( wxString name, work->mRemove )
        {
            BOOST_FOREACH( NodePtr child, getChildren() )
            {
                if (child->getName().IsSameAs(name))
                {
                    nodes.push_back(child);
                    break;
                }
            }
        }
        removeChildren(nodes);// All at once, for better performance (less UI updates)
    }
    if (!work->mDirExists)
    {
        // Directory was removed. Remove the node also.
        if (!hasParent())
        {
            // Node was already removed from parent (maybe parent dir deleted also?)
        }
        else
        {
            if (!getParent()->isA<AutoFolder>())
            {
                getParent()->removeChild(shared_from_this()); // Do not execute after the dialog (crash can occur then - since the node is removed by another update)
                gui::Dialog::get().getConfirmation(_("Folder removed"), _("The folder ") + util::path::toName(getPath()) + _(" has been removed from disk. Folder is removed from project also."));
            }
        }
    }
    else
    {
        if (mUpdateAgain)
        {
            check();
        }
    }
    mUpdateAgain = false;
}

//////////////////////////////////////////////////////////////////////////
// ATTRIBUTES
//////////////////////////////////////////////////////////////////////////

wxString AutoFolder::getName() const
{
    if (getParent()->isA<AutoFolder>())
    {
        return util::path::toName(mPath);
    }
    return util::path::toPath(mPath);

}

time_t AutoFolder::getLastModified() const
{
    return mLastModified;
}

wxString AutoFolder::getSequenceName() const
{
    return util::path::toName(mPath);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AutoFolder::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Folder>(*this);
    ar & mPath;
    ar & mLastModified;
}
template void AutoFolder::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AutoFolder::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace