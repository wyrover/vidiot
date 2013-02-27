#include "ChangeVideoClipTransform.h"

#include "Enums.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ChangeVideoClipTransform::ChangeVideoClipTransform(model::VideoClipPtr videoclip)
    :   RootCommand()
    ,   mInitialized(false)
    ,   mVideoClip(videoclip)
    ,   mOldOpacity(mVideoClip->getOpacity())
    ,   mOldScaling(mVideoClip->getScaling())
    ,   mOldScalingFactor(mVideoClip->getScalingFactor())
    ,   mOldAlignment(mVideoClip->getAlignment())
    ,   mOldPosition(mVideoClip->getPosition())
    ,   mNewOpacity(boost::none)
    ,   mNewScaling(boost::none)
    ,   mNewScalingFactor(boost::none)
    ,   mNewPosition(boost::none)
{
}

ChangeVideoClipTransform::~ChangeVideoClipTransform()
{
}

void ChangeVideoClipTransform::setOpacity(int opacity)
{
    mNewOpacity = boost::optional<int>(opacity);
    mVideoClip->setOpacity(opacity);
}

void ChangeVideoClipTransform::setScaling(VideoScaling scaling, boost::optional< boost::rational< int > > factor)
{
    mNewScaling = boost::optional<VideoScaling>(scaling);
    mNewScalingFactor = factor;
    mVideoClip->setScaling(*mNewScaling, mNewScalingFactor);
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
        if (mNewOpacity)
        {
            mVideoClip->setOpacity(*mNewOpacity);
        }
        if (mNewScaling)
        {
            mVideoClip->setScaling(*mNewScaling, mNewScalingFactor);
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
    if (mNewOpacity)
    {
        mVideoClip->setOpacity(mOldOpacity);
    }
    if (mNewScaling)
    {
        mVideoClip->setScaling(mOldScaling, boost::optional< boost::rational< int > >(mOldScalingFactor));
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
        << obj.mOldOpacity       << '|'
        << obj.mOldScaling       << '|'
        << obj.mOldScalingFactor << '|'
        << obj.mOldAlignment     << '|'
        << obj.mOldPosition      << '|'
        << obj.mNewOpacity       << '|'
        << obj.mNewScaling       << '|'
        << obj.mNewScalingFactor << '|'
        << obj.mNewPosition;
    return os;
}

} // namespace