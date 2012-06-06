#include "VideoParameters.h"

#include "UtilLogWxwidgets.h"

// todo move ALL video classes to modelvideo

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoParameters::VideoParameters()
    :   mBoundingBox(0,0)
    ,   mDrawBoundingBox(false)
{
}

VideoParameters::VideoParameters(const VideoParameters& other)
    :   mBoundingBox(other.mBoundingBox)
    ,   mDrawBoundingBox(other.mDrawBoundingBox)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoParameters& VideoParameters::setBoundingBox(wxSize boundingBox)
{
    mBoundingBox = boundingBox;
    return *this;
}

wxSize VideoParameters::getBoundingBox() const
{
    return mBoundingBox;
}

VideoParameters& VideoParameters::setDrawBoundingBox()
{
    mDrawBoundingBox = true;
    return *this;
}

bool VideoParameters::getDrawBoundingBox() const
{
    return mDrawBoundingBox;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoParameters& obj )
{
    os << &obj << '|' << obj.mBoundingBox << '|' << obj.mDrawBoundingBox;
    return os;
}

} //namespace