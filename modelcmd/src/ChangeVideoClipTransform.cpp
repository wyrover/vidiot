#include "ChangeVideoClipTransform.h"

#include "Enums.h"
#include "UtilLog.h"
#include "VideoClip.h"

namespace model {

class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ChangeVideoClipTransform::ChangeVideoClipTransform(model::VideoClipPtr videoclip)
    :   RootCommand()
    ,   mInitialized(false)
    ,   mVideoClip(videoclip)
    ,   mOldScaling(mVideoClip->getScaling())
    ,   mOldScalingFactor(mVideoClip->getScalingFactor())
    ,   mOldAlignment(mVideoClip->getAlignment())
    ,   mOldRegionOfInterest(mVideoClip->getRegionOfInterest())
    ,   mNewScaling(boost::none)
    ,   mNewScalingFactor(boost::none)
{
}

ChangeVideoClipTransform::~ChangeVideoClipTransform()
{
}

// todo rename all commands to *Command?

void ChangeVideoClipTransform::setScaling(VideoScaling scaling, boost::optional<double> scalingfactor)
{
    mNewScaling = boost::optional<VideoScaling>(scaling);
    mNewScalingFactor = scalingfactor;
    if (mNewScaling)
    {
        mVideoClip->setScaling(*mNewScaling, mNewScalingFactor);
    }
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool ChangeVideoClipTransform::Do()
{
    VAR_INFO(*this)(mInitialized);

    if (mInitialized)
    {
        // Only the second time that Do() is called (redo) something actually needs to be done.
        // The first time is handled by (possibly multiple calls to) setScaling etc.
        if (mNewScaling)
        {
            mVideoClip->setScaling(*mNewScaling, mNewScalingFactor);
        }
    }
    mInitialized = true;
    return true;
}

bool ChangeVideoClipTransform::Undo()
{
    VAR_INFO(*this);
    if (mNewScaling)
    {
        mVideoClip->setScaling(mOldScaling, boost::optional<double>(mOldScalingFactor));
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::VideoClipPtr ChangeVideoClipTransform::getVideoClip() const
{
    return mVideoClip;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ChangeVideoClipTransform& obj )
{
    os << &obj << '|' << typeid(obj).name() << '|' << obj.mVideoClip;
    return os;
}

} // namespace