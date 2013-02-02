#include "AutoFolder.h"

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

namespace model {

//////////////////////////////////////////////////////////////////////////
// WORK OBJECT FOR INDEXING A FOLDER
//////////////////////////////////////////////////////////////////////////

struct IndexAutoFolderWork
    : public Work
{
    explicit IndexAutoFolderWork(AutoFolderPtr folder, bool recurse = true)
        : Work(boost::bind(&IndexAutoFolderWork::indexFiles,this))
        , mFolder(folder)
        , mRecurse(recurse)
    {
    }

    void indexFiles()
    {
        wxFileName path = mFolder->getPath();
        ASSERT(path.IsDir() && path.IsAbsolute())(path);
        wxString pathname =
            path.GetDirCount() >= 1 ? path.GetDirs().Last() + "/" :
            path.HasVolume() ? path.GetVolume() :
            "";
        gui::StatusBar::get().setProcessingText(_("Updating '") + pathname + "'");

        // Fill 'allcurrententries' with current list of children.
        std::list<wxString> allcurrententries;
        BOOST_FOREACH( NodePtr child, mFolder->getChildren() )
        {
            allcurrententries.push_back(child->getName());
        }
        NodePtrs newnodes;
        wxDir dir( path.GetLongPath() );
        ASSERT(dir.IsOpened());
        wxString nodename;

        wxArrayString allfiles;
        size_t count = wxDir::GetAllFiles(path.GetLongPath(), &allfiles, wxEmptyString, wxDIR_FILES);
        gui::StatusBar::get().showProgressBar(count);
        int progress = 0;
        // Find all subfolders
        for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_DIRS); cont; cont = dir.GetNext(&nodename))
        {
            if (UtilList<wxString>(allcurrententries).hasElement(nodename)) // Existing element. Do not remove
            {
                UtilList<wxString>(allcurrententries).removeElements(boost::assign::list_of(nodename));
            }
            else // New element. Add
            {
                wxFileName filename(path.GetLongPath(), "");
                filename.AppendDir(nodename);
                ASSERT(filename.IsDir());
                AutoFolderPtr folder = boost::make_shared<AutoFolder>(filename);
                newnodes.push_back(folder);
                if (mRecurse)
                {
                    folder->update(true);
                }
            }
            gui::StatusBar::get().showProgress(++progress);
        }

        // Find all files
        for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_FILES); cont; cont = dir.GetNext(&nodename))
        {
            if (UtilList<wxString>(allcurrententries).hasElement(nodename)) // Existing element. Do not remove
            {
                UtilList<wxString>(allcurrententries).removeElements(boost::assign::list_of(nodename));
            }
            else // New element. Add
            {
                wxFileName filename(path.GetLongPath(), nodename);
                gui::StatusBar::get().setProcessingText(_("Updating '") + pathname + nodename + "'");
                model::FilePtr file = boost::make_shared<model::File>(filename);
                if (file->canBeOpened())
                {
                    newnodes.push_back(file);
                }
            }
            gui::StatusBar::get().showProgress(++progress);
        }

        mFolder->addChildren(newnodes); // Add all at once, for better performance (less UI updates)

         // Remove all other elements (these have been removed).
        BOOST_FOREACH( wxString name, allcurrententries )
        {
            BOOST_FOREACH( NodePtr child, mFolder->getChildren() )
            {
                if (!child->getName().IsSameAs(name))
                {
                    mFolder->removeChild(child);
                    break;
                }
            }
        }

        gui::StatusBar::get().hideProgressBar();
        gui::StatusBar::get().setProcessingText("");
    }

    AutoFolderPtr mFolder;
    bool mRecurse; ///< If true, recurse into subdirs
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AutoFolder::AutoFolder()
:   Folder()
,   mPath()
,   mLastModified(0)
{
    VAR_DEBUG(this);
}

AutoFolder::AutoFolder(wxFileName path)
:   Folder(util::path::toName(path))
,   mPath(util::path::normalize(path))
,   mLastModified(mPath.GetModificationTime().GetTicks())
{
    ASSERT(path.IsDir())(path);
    VAR_DEBUG(this);
}

AutoFolder::~AutoFolder()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IPATH
//////////////////////////////////////////////////////////////////////////

wxFileName AutoFolder::getPath() const
{
    return mPath;
}

//////////////////////////////////////////////////////////////////////////
// STRUCTURE
//////////////////////////////////////////////////////////////////////////

void AutoFolder::update(bool recurse)
{
    gui::Worker::get().schedule(boost::shared_ptr<Work>(new IndexAutoFolderWork(boost::dynamic_pointer_cast<AutoFolder>(shared_from_this()), recurse)));
    return;
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
    return mPath.GetFullPath();
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