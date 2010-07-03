#ifndef TIMELINE_MOVE_CLIPS_H
#define TIMELINE_MOVE_CLIPS_H

#include "TimelineCommand.h"
#include <set>
#include "ModelPtr.h"

namespace command {

class TimelineMoveClips : public TimelineCommand
{
public:
    struct MoveParameter
    {
        /** Into this track the moved clips need to be inserted. */
        model::TrackPtr addTrack;

        /**
        * The moved clips must be inserted before this clip.
        * If this is an uninitialized pointer, then the clips need
        * to be inserted at the end of the track. 
        */
        model::ClipPtr addPosition;

        /**
        * Consecutive list of clips to be added to this track
        * (they'll be joined together exactly in the same order). 
        * These clips may also be a part of removeClips.
        * If multiple consecutive (but not directly connected) lists
        * need to be added, add multiple MoveParameter objects.
        */
        model::Clips addClips;

        /** From this track the moved clips need to be removed. */
        model::TrackPtr removeTrack;

        /**
        * In case of undo, the removed clips must be reinserted
        * before this clip.If this is an uninitialized pointer, 
        * then the clips need to be inserted at the end of 
        * the track. 
        */
        model::ClipPtr removePosition;

        /**
        * Any clips to be removed from this track 
        * These clips may also be a part of addClips.
        * This needs to be a consecutive list of clips, 
        * thus without 'emptyness' in between. If multiple,
        * not directly connected clip lists need to be removed,
        * instantiate two MoveParameter objects (one for each
        * list).
        */
        model::Clips removeClips;
    };
    typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
    typedef std::set<MoveParameterPtr> MoveParameters;

    TimelineMoveClips(MoveParameters params);
    ~TimelineMoveClips();
    bool Do();
    bool Undo();
private:
    MoveParameters mParams;
    MoveParameters mParamsUndo;

    void doMove(MoveParameterPtr move);
};

} // namespace

#endif // TIMELINE_MOVE_CLIPS_H
