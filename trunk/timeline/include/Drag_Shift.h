#ifndef DRAG_SHIFT_H
#define DRAG_SHIFT_H

#include <boost/optional.hpp>
#include "UtilInt.h"

namespace gui { namespace timeline {
/// Helper class for administering a shift during the DND operation
struct ShiftParams
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ShiftParams(pts position, pts length);
    ShiftParams(const ShiftParams& other);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mPosition;  ///< Position at which clips must be shifted to make room for the clips being dragged.
    pts mLength;    ///< Length of the shift required to make room for the clips being dragged.
};

std::ostream& operator<< (std::ostream& os, const ShiftParams& obj);
bool operator== ( ShiftParams const& x, ShiftParams const& y );
bool operator!= ( ShiftParams const& x, ShiftParams const& y );

typedef boost::optional<ShiftParams> Shift;
}} // namespace

#endif