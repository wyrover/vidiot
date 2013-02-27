#include "VideoCompositionParameters.h"

#include "UtilLogWxwidgets.h"
#include "Config.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters::VideoCompositionParameters()
    :   mBoundingBox(0,0)
    ,   mDrawBoundingBox(false)
{
}

VideoCompositionParameters::VideoCompositionParameters(const VideoCompositionParameters& other)
    :   mBoundingBox(other.mBoundingBox)
    ,   mDrawBoundingBox(other.mDrawBoundingBox)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters& VideoCompositionParameters::setBoundingBox(wxSize boundingBox)
{
    mBoundingBox = boundingBox;
    ASSERT_MORE_THAN_ZERO(mBoundingBox.GetWidth());
    ASSERT_MORE_THAN_ZERO(mBoundingBox.GetHeight());
    ASSERT_MORE_THAN_EQUALS_ZERO(mBoundingBox.GetX());
    ASSERT_MORE_THAN_EQUALS_ZERO(mBoundingBox.GetY());
    return *this;
}

wxSize VideoCompositionParameters::getBoundingBox() const
{
    return mBoundingBox;
}

VideoCompositionParameters& VideoCompositionParameters::setDrawBoundingBox(bool draw)
{
    mDrawBoundingBox = draw;
    return *this;
}

bool VideoCompositionParameters::getDrawBoundingBox() const
{
    return mDrawBoundingBox;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoCompositionParameters& obj )
{
    os << &obj << '|' << obj.mBoundingBox << '|' << obj.mDrawBoundingBox;
    return os;
}

} //namespace