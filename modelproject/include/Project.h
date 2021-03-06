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

#include "UtilSingleInstance.h"

namespace model {

class FileMetaDataCache;

/// \class Project
/// This class is not managed via shared_ptr's since it's construction/destruction
/// is managed by the wxWidgets document/view framework. Therefore, pointer ownership
/// cannot be transferred to a shared_ptr. This was also the rational to not let
/// Project inherit from ProjectAsset, and let the document be the root node. Instead,
/// the document has a separate root node 'mRoot'.
/// \image html Project.png
class Project
    :   public wxDocument
    ,   public SingleInstance<Project>
{
public:

    static const wxString sFileExtension;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Project();
    virtual ~Project();

    //////////////////////////////////////////////////////////////////////////
    // OVERRIDES - wxDocument
    //////////////////////////////////////////////////////////////////////////

    /// Required for destruction (closing) of the project, because the lifetime
    /// of the project is handled by wxWidgets. Furthermore, the project tree
    /// (shared_ptr's to 'Node') is not automatically cleaned up
    /// upon destruction since parents and children keep references to each other.
    /// This method ensures that all these bidirectional references are removed.
    bool DeleteContents() override;
    bool OnCloseDocument() override;
    bool OnNewDocument() override;
    bool OnCreate(const wxString& path, long flags) override;
    wxCommandProcessor* OnCreateCommandProcessor() override;
    void OnChangeFilename(bool notifyViews) override;

    //////////////////////////////////////////////////////////////////////////
    // LOAD/SAVE - wxDocument
    //////////////////////////////////////////////////////////////////////////

    std::ostream& SaveObject(std::ostream& stream) override;
    std::istream& LoadObject(std::istream& stream) override;

    static wxFileName createBackupFileName(wxFileName input, int count);

    /// Overridden to change the error dialogs.
    bool DoSaveDocument(const wxString& file) override;

    /// Overridden to change the error dialogs.
    bool DoOpenDocument(const wxString& file) override;

    /// Overridden to first close the project when reverting
    bool Revert() override;

    //////////////////////////////////////////////////////////////////////////
    // ACCESSORS
    //////////////////////////////////////////////////////////////////////////

    FolderPtr getRoot() const;

    wxString getName() const;

    wxFileName convertPathForSaving(const wxFileName& path) const;
    wxFileName convertPathAfterLoading(const wxFileName& path) const;

private:

    FolderPtr mRoot;
    boost::shared_ptr<FileMetaDataCache> mMetaDataCache;
    PropertiesPtr mProperties;
    wxString mSaveFolder;
    bool mReverting;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    DECLARE_DYNAMIC_CLASS(Project)

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::Project, 2)
BOOST_CLASS_EXPORT_KEY(model::Project)
