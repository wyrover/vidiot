#ifndef IDS_H
#define IDS_H

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
    // Timelne popup menu
    meID_ADD_TRANSITION,
    meID_REMOVE_EMPTY,
};

#endif // IDS_H
