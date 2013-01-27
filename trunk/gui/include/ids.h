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
    // Sequence menu
    ID_ADDVIDEOTRACK,
    ID_ADDAUDIOTRACK,
    ID_REMOVE_EMPTY_TRACKS,
    ID_DELETEMARKED,
    ID_DELETEUNMARKED,
    ID_REMOVEMARKERS,
    ID_RENDERSEQUENCE,
    ID_RENDERSETTINGS,
    ID_RENDERALL,
    ID_CLOSESEQUENCE,
    // Options menu
    ID_OPTIONS,
    // Project view popup menu
    meID_NEW_FOLDER,
    meID_NEW_AUTOFOLDER,
    meID_NEW_SEQUENCE,
    meID_NEW_FILE,
    meID_CREATE_SEQUENCE,
    // Timeline popup menu
    meID_ADD_INTRANSITION,
    meID_ADD_INOUTTRANSITION,
    meID_ADD_OUTTRANSITION,
    meID_ADD_INFADE,
    meID_ADD_INOUTFADE,
    meID_ADD_OUTFADE,
    meID_REMOVE_EMPTY,
    // Entries that can be dynamically generated
    meID_DYNAMIC_MENU_ENTRIES,
    meID_VIDEOTRANSITIONS_BEGIN = meID_DYNAMIC_MENU_ENTRIES  + 1000,
    meID_AUDIOTRANSITIONS_BEGIN = meID_VIDEOTRANSITIONS_BEGIN + 1000,     // 1000 video transitions
    meID_DYNAMIC_MENU_ENTRIES_END = meID_AUDIOTRANSITIONS_BEGIN + 1000,  // 1000 audio transitions
    // Should always be the last value!!!
    meID_LAST
};

#endif // IDS_H