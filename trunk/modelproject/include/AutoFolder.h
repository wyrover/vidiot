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

#ifndef AUTO_FOLDER_H
#define AUTO_FOLDER_H

#include "Folder.h"
#include "IPath.h"

namespace worker {
    class WorkDoneEvent;
}

namespace model {

struct IndexAutoFolderWork;

class AutoFolder
    :   public Folder
    ,   public IPath
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Recovery constructor.
    AutoFolder();

    /// \param path full path to the folder.
    AutoFolder(const wxFileName& path);

    virtual ~AutoFolder();

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    NodePtrs findPath(const wxString& path) override;
    bool mustBeWatched(const wxString& path) override;
    void check(bool immediately = false) override; ///< Update the autofolder children. The folder is synced with the filesystem.

    //////////////////////////////////////////////////////////////////////////
    // IPATH
    //////////////////////////////////////////////////////////////////////////

    wxFileName getPath() const override;

    /// Called when the updating is done
    void onWorkDone(worker::WorkDoneEvent& event);

    void handleWorkDone(boost::shared_ptr<IndexAutoFolderWork> work, bool immediately);

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    /// Returns the full path if the parent node is not an AutoFolder. Returns
    /// the relatve path only if the parent node is an AutoFolder.
    wxString getName() const override;

    wxString getSequenceName() const override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxFileName mPath;
    boost::shared_ptr<IndexAutoFolderWork> mCurrentUpdate;
    bool mUpdateAgain; ///< True if an update event was received while an update was already scheduled. To avoid a new file being found twice, no two updates are scheduled simultaneously.

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::AutoFolder, 2)
BOOST_CLASS_EXPORT_KEY(model::AutoFolder)

#endif
