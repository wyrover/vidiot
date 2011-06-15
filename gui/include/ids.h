#ifndef IDS_H
#define IDS_H

/// Made global list to be able to trigger these from the tests
enum 
{
    ID_FIRST = wxID_HIGHEST + 1,
    // View menu
    ID_SNAP_CLIPS,
    ID_SNAP_CURSOR,
    // Sequence menu
    ID_ADDVIDEOTRACK,
    ID_ADDAUDIOTRACK,
    ID_DELETEMARKED,
    ID_DELETEUNMARKED,
    ID_REMOVEMARKERS,
    ID_CLOSESEQUENCE,
    // Options menu
    ID_OPTIONS,
    // Project view popup menu
    meID_NEW_FOLDER,
    meID_NEW_AUTOFOLDER,
    meID_NEW_SEQUENCE,
    meID_NEW_FILE,
    meID_CREATE_SEQUENCE,
    meID_UPDATE_AUTOFOLDER,
    // Timeline popup menu
    meID_ADD_TRANSITION,
    meID_REMOVE_EMPTY,
};

#endif // IDS_H
