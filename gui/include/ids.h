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

/// Made global list to be able to trigger these from the tests
enum
{
    ID_FIRST = wxID_HIGHEST + 1,
    // Project menu
    ID_NEW_FILES,
    ID_NEW_AUTOFOLDER,
    ID_NEW_SEQUENCE,
    ID_NEW_FOLDER,
    // View menu
    ID_SHOW_PROJECT,
    ID_SHOW_DETAILS,
    ID_SHOW_PREVIEW,
    ID_SHOW_TIMELINES,
    // View menu -> Details
    ID_DETAILS_SHOW_LABELS,
    ID_DETAILS_SHOW_ICONS,
    // View menu -> Preview
    ID_PREVIEW_SHOW_BOUNDINGBOX,
    // View menu -> Timeline
    ID_TIMELINE_SNAP_CLIPS,
    ID_TIMELINE_SNAP_CURSOR,
    // Sequence menu
    ID_ADDVIDEOTRACK,
    ID_ADDAUDIOTRACK,
    ID_REMOVE_EMPTY_TRACKS,
    ID_DELETEMARKED,
    ID_DELETEUNMARKED,
    ID_DELETEEMPTY,
    ID_REMOVEMARKERS,
    ID_RENDERSEQUENCE,
    ID_RENDERSETTINGS,
    ID_RENDERALL,
    ID_CLOSESEQUENCE,
    // Workspace menu
    ID_WORKSPACE_SHOW_CAPTIONS,
    ID_WORKSPACE_SAVE,
    ID_WORKSPACE_LOAD,
    ID_WORKSPACE_DELETE,
    ID_WORKSPACE_DELETEALL,
    ID_WORKSPACE_DEFAULT,
    ID_WORKSPACE_FULLSCREEN,
    // Help menu
    ID_OPENLOGFILE,
    ID_OPENCONFIGFILE,
    ID_SUBMITBUG,
    // Project view popup menu specific
    ID_CREATE_SEQUENCE,
    ID_DELETE_UNUSED,
    // Should always be the last value!!!
    meID_LAST
};
