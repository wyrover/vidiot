// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef IDS_H
#define IDS_H

/// Made global list to be able to trigger these from the tests
enum
{
    ID_FIRST = wxID_HIGHEST + 1,
    // View menu
    ID_SNAP_CLIPS,
    ID_SNAP_CURSOR,
    ID_SHOW_BOUNDINGBOX,
    ID_SHOW_PROJECT,
    ID_SHOW_DETAILS,
    ID_SHOW_PREVIEW,
    ID_SHOW_TIMELINES,
    // Sequence menu
    ID_ADDVIDEOTRACK,
    ID_ADDAUDIOTRACK,
    ID_REMOVE_EMPTY_TRACKS,
    ID_SPLIT_AT_CURSOR,
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
    // Should always be the last value!!!
    meID_LAST
};

#endif // IDS_H