// Copyright 2013 Eric Raijmakers.
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

#ifndef PROJECT_VIEW_DROP_SOURCE_H
#define PROJECT_VIEW_DROP_SOURCE_H

#include "UtilSingleInstance.h"

namespace model {
class INode;
typedef INode* NodeId;
}

namespace gui {

class ProjectViewModel;
class DataObject;

class ProjectViewDropSource
    :   public wxDropSource
    ,   public SingleInstance<ProjectViewDropSource>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectViewDropSource(wxDataViewCtrl& ctrl, ProjectViewModel& model);
    ~ProjectViewDropSource();

    //////////////////////////////////////////////////////////////////////////
    // FROM WXDROPSOURCE
    //////////////////////////////////////////////////////////////////////////

    virtual bool GiveFeedback(wxDragResult effect) override;

    //////////////////////////////////////////////////////////////////////////
    // DRAGGING
    //////////////////////////////////////////////////////////////////////////

    void startDrag(DataObject& data);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    DataObject& getData();       ///< \return the current associated data object
    void setFeedback(bool enabled); ///< Enable/disable showing feedback. Used to avoid flicker

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    bool isDragActive() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxDataViewCtrl& mCtrl;
    ProjectViewModel& mModel;
    wxFrame *mHint;
    bool mFeedback;             ///< true if feedback must be shown while drawing
    bool mActive;               ///< true if a drag operation has been started and not yet aborted

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void drawAsset(wxDC* dc, wxRect rect, model::NodePtr asset);

};

} // namespace

#endif // PROJECT_VIEW_DROP_SOURCE_H