#include "VideoCompositionParameters.h"

#include "UtilLogWxwidgets.h"
#include "Config.h"

// todo move ALL video classes to modelvideo

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters::VideoCompositionParameters()
    :   mBoundingBox(0,0)
    ,   mDrawBoundingBox(Config::ReadBool(Config::sPathShowBoundingBox))
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