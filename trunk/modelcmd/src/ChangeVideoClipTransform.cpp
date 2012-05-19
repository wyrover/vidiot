#include "ChangeVideoClipTransform.h"

#include "Enums.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"

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
    ,   moldScalingDigits(mVideoClip->getScalingDigits())
    ,   mOldAlignment(mVideoClip->getAlignment())
    ,   mOldPosition(mVideoClip->getPosition())
    ,   mNewScaling(boost::none)
    ,   mNewScalingDigits(boost::none)
    ,   mNewPosition(boost::none)
{
}

ChangeVideoClipTransform::~ChangeVideoClipTransform()
{
}

void ChangeVideoClipTransform::setScaling(VideoScaling scaling, boost::optional<int> scalingdigits)
{
    mNewScaling = boost::optional<VideoScaling>(scaling);
    mNewScalingDigits = scalingdigits;
    mVideoClip->setScaling(*mNewScaling, mNewScalingDigits);
}

void ChangeVideoClipTransform::setAlignment(VideoAlignment alignment)
{
    mNewAlignment = boost::optional<VideoAlignment>(alignment);
    mVideoClip->setAlignment(*mNewAlignment);
}

void ChangeVideoClipTransform::setPosition(wxPoint position)
{
    mNewPosition = boost::optional<wxPoint>(position);
    mVideoClip->setPosition(*mNewPosition);
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
            mVideoClip->setScaling(*mNewScaling, mNewScalingDigits);
        }
        if (mNewAlignment)
        {
            mVideoClip->setAlignment(*mNewAlignment);
        }
        if (mNewPosition)
        {
            mVideoClip->setPosition(*mNewPosition);
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
        mVideoClip->setScaling(mOldScaling, boost::optional<int>(moldScalingDigits));
    }
    if (mNewAlignment)
    {
        mVideoClip->setAlignment(mOldAlignment);
    }
    if (mNewPosition)
    {
        mVideoClip->setPosition(mOldPosition);
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
    os  << &obj << '|'
        << typeid(obj).name()    << '|'
        << obj.mVideoClip        << '|'
        << obj.mOldScaling       << '|'
        << obj.moldScalingDigits << '|'
        << obj.mOldAlignment     << '|'
        << obj.mOldPosition      << '|'
        << obj.mNewScaling       << '|'
        << obj.mNewScalingDigits << '|'
        << obj.mNewPosition;
    return os;
}

} // namespace