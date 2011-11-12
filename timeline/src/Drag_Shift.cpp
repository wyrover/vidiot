#include "Drag_Shift.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ShiftParams::ShiftParams(pts position, pts length)
    : mPosition(position)
    , mLength(length) 
{
}

ShiftParams::ShiftParams(const ShiftParams& other) 
    : mPosition(other.mPosition)
    , mLength(other.mLength) 
{
}

//////////////////////////////////////////////////////////////////////////
// COMPARISON
//////////////////////////////////////////////////////////////////////////

bool operator== ( ShiftParams const& x, ShiftParams const& y )
{
    return (x.mPosition == y.mPosition) && (x.mLength == y.mLength);
}

bool operator!= ( ShiftParams const& x, ShiftParams const& y )
{
    return (x.mPosition != y.mPosition) || (x.mLength != y.mLength);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const ShiftParams& obj)
{
    os << obj.mPosition << '|' << obj.mLength;
    return os;
}

}} // namespace

