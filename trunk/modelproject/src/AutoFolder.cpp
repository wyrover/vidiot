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

#include "AutoFolder.h"

#include "Dialog.h"
#include "File.h"
#include "Project.h"
#include "StatusBar.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilVector.h"
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
    explicit IndexAutoFolderWork(const AutoFolderPtr& folder)
        : worker::Work(boost::bind(&IndexAutoFolderWork::indexFiles,this))
        , mFolder(folder)
        , mPath(folder->getPath())
    {
        ASSERT(mPath.IsDir() && mPath.IsAbsolute())(mPath);
        for ( NodePtr child : mFolder->getChildren() )
        {
            mRemove.push_back(child->getName());
        }
    }

    void indexFiles()
    {
        if (!wxThread::IsMain()) { setThreadName("IndexFiles"); }
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

            // Find all files
            for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_FILES); cont; cont = dir.GetNext(&nodename))
            {
                if (isAborted()) { return; }
                if (UtilVector<wxString>(mRemove).hasElement(nodename)) // Existing element. Do not remove
                {
                    UtilVector<wxString>(mRemove).removeElements(boost::assign::list_of(nodename));
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

            if (wxThread::IsMain())
            {
                // In case this indexing is called directly from the main thread (instead of being
                // executed by a worker), then the progress bar must be explicitly cleared.
                gui::StatusBar::get().hideProgressBar();
                gui::StatusBar::get().setProcessingText("");
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

    AutoFolderPtr mFolder;                 ///< Folder to be indexed
    bool mDirExists;                       ///< true if the dir could be opened, false if not (then, assume that it was removed)
    wxFileName mPath;                      ///< Path to the folder to be indexed
    std::vector<wxString> mCurrentEntries; ///< All entries known when the work was scheduled
    NodePtrs mAdd;                         ///< When done, holds the list of nodes that must be added
    std::vector<wxString> mRemove;         ///< All entries to be removed when indexing is done
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AutoFolder::AutoFolder()
    :   Folder()
    ,   mPath()
    ,   mCurrentUpdate()
    ,   mUpdateAgain(false)
{
    VAR_DEBUG(this);
}

AutoFolder::AutoFolder(const wxFileName& path)
    :   Folder(util::path::toName(path))
    ,   mPath(util::path::normalize(path))
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

NodePtrs AutoFolder::findPath(const wxString& path)
{
    NodePtrs result = Node::findPath(path); // Traverse the tree
    if (util::path::equals(mPath,path))
    {
        result.push_back(self());
    }
    return result;
}

bool AutoFolder::mustBeWatched(const wxString& path)
{
    if (util::path::isParentOf(path,mPath))
    {
        // Yes, is parent folder of this folder
        return true;
    }
    return Node::mustBeWatched(path); // Maybe for any of the children?
}

void AutoFolder::check(bool immediately)
{
    ASSERT(wxThread::IsMain());
    ASSERT_IMPLIES(getParent(), !getParent()->isA<model::AutoFolder>());
    if (immediately)
    {
        boost::shared_ptr<IndexAutoFolderWork>  work = boost::make_shared<IndexAutoFolderWork>(boost::dynamic_pointer_cast<AutoFolder>(self()));
        work->indexFiles();
        handleWorkDone(work,true);
    }
    else
    {
        if (mCurrentUpdate)
        {
            mUpdateAgain = true; // When mCurrentUpdate is finished, another will be scheduled.
            VAR_DEBUG(mPath)(mUpdateAgain);
        }
        else
        {
            VAR_DEBUG(mPath);
            mCurrentUpdate = boost::make_shared<IndexAutoFolderWork>(boost::dynamic_pointer_cast<AutoFolder>(self()));
            mCurrentUpdate->Bind(worker::EVENT_WORK_DONE, &AutoFolder::onWorkDone, this); // No unbind: work object is destroyed when done
            worker::VisibleWorker::get().schedule(mCurrentUpdate);
        }
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
    handleWorkDone(work,false);
}

void AutoFolder::handleWorkDone(boost::shared_ptr<IndexAutoFolderWork> work, bool immediately)
{
    if (!work->mAdd.empty())
    {
        addChildren(work->mAdd); // Add all at once, for better performance (less UI updates)
        for ( NodePtr node : work->mAdd )
        {
            AutoFolderPtr autoFolder = boost::dynamic_pointer_cast<AutoFolder>(node);
            if (autoFolder)
            {
                autoFolder->check(immediately);
            }
        }
    }

    if (!work->mRemove.empty())
    {
        model::NodePtrs nodes;
        for ( wxString name : work->mRemove )
        {
            for ( NodePtr child : getChildren() )
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
                getParent()->removeChild(self()); // Do not execute after the dialog (crash can occur then - since the node is removed by another update)
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
    if (getParent() &&
        getParent()->isA<AutoFolder>())
    {
        return util::path::toName(mPath);
    }
    return util::path::toPath(mPath);

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
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Folder);
        if (Archive::is_loading::value)
        {
            wxFileName path;
            ar & boost::serialization::make_nvp( "mPath", path );
            mPath = model::Project::get().convertPathAfterLoading(path);
        }
        else
        {
            wxFileName path = model::Project::get().convertPathForSaving(mPath);
            ar & boost::serialization::make_nvp( "mPath", path );
        }
        if (version == 1)
        {
            time_t mLastModified;
            ar & BOOST_SERIALIZATION_NVP(mLastModified);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AutoFolder::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AutoFolder::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AutoFolder)
