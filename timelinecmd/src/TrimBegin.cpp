#include "TrimBegin.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "Timeline.h"
#include "Cursor.h"
#include "Track.h"
#include "Sequence.h"
#include "Zoom.h"
#include "EmptyClip.h"
#include "Clip.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimBegin::TrimBegin(gui::timeline::Timeline& timeline, model::ClipPtr clip, pts diff)
    :   AClipEdit(timeline)
    ,   mClip(clip)
    ,   mDiff(diff)
{
    VAR_INFO(this);
    mCommandName = _("Adjust clip begin point");
}

TrimBegin::~TrimBegin()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimBegin::initialize()
{
    VAR_INFO(this);
    model::ClipPtr fillClip;
    model::ClipPtr fillLink;
    // NIY: What if the link is 'shifted' wrt original clip?
    ASSERT(mClip->getLeftPts() == mClip->getLink()->getLeftPts());

    model::ClipPtr newclip = make_cloned<model::Clip>(mClip);
    newclip->adjustBegin(mDiff);
    model::ClipPtr newlink = make_cloned<model::Clip>(mClip->getLink());
    newlink->adjustBegin(mDiff);

    ReplacementMap linkmapper;
    model::Clips replace = boost::assign::list_of(newclip);
    model::Clips replacelink = boost::assign::list_of(newlink);

    if (mDiff > 0)
    {
        // Add empty clip in front of new clip: new clip is shorter than original clip
        // and the frames should maintain at the same position.
        replace.push_front(boost::static_pointer_cast<model::Clip>(boost::make_shared<model::EmptyClip>(mDiff)));
        replacelink.push_front(boost::static_pointer_cast<model::Clip>(boost::make_shared<model::EmptyClip>(mDiff)));
    }
    replaceClip(mClip, replace, &linkmapper);
    replaceClip(mClip->getLink(), replacelink, &linkmapper);
    replaceLinks(linkmapper);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TrimBegin::splittrack(model::Tracks tracks, pts position, ReplacementMap& linkmapper)
{
    BOOST_FOREACH( model::TrackPtr track, tracks )
    {
        split(track, position, &linkmapper);
    }
}

}}} // namespace
