#include "TimelineMoveClips.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "UtilLog.h"
#include "Track.h"

namespace command {

TimelineMoveClips::TimelineMoveClips(gui::timeline::GuiTimeLine& timeline, model::MoveParameters params)
:   TimelineCommand(timeline)
,   mParams(params)
{
    VAR_INFO(this);
    mCommandName = _("Move clips");
}

TimelineMoveClips::~TimelineMoveClips()
{
}

bool TimelineMoveClips::Do()
{
    VAR_INFO(this);

    bool redo = (mParamsUndo.size() != 0);

    BOOST_FOREACH( model::MoveParameterPtr move, mParams )
    {
        if (!redo)
        {
            // Save undo information
            //model::MoveParameterPtr undo = boost::make_shared<model::MoveParameter>();
            //undo->addTrack          = move->removeTrack;
            //undo->addPosition       = move->removePosition;
            //undo->addClips          = move->removeClips;
            //undo->removeTrack       = move->addTrack;
            //undo->removePosition    = move->addPosition;
            //undo->removeClips       = move->addClips;
            mParamsUndo.push_back(boost::make_shared<model::MoveParameter>(
                move->removeTrack, 
                move->removePosition, 
                move->removeClips, 
                move->addTrack, 
                move->addPosition, 
                move->addClips));
        }
        doMove(move);
    }
    return true;
}

bool TimelineMoveClips::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( model::MoveParameterPtr move, mParamsUndo )
    {
        doMove(move);
    }
    return true;
}

void TimelineMoveClips::doMove(model::MoveParameterPtr move)
{
    // First, remove clips
    if (move->removeClips.size() > 0)
    {
        move->removeTrack->removeClips(move->removeClips);
    }

    // Second, add clips
    if (move->addClips.size() > 0)
    {
        move->addTrack->addClips(move->addClips,move->addPosition);
    }
}

} // namespace