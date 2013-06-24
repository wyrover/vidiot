#ifndef HELPER_TRIM_H
#define HELPER_TRIM_H

#include <boost/shared_ptr.hpp>
#include <wx/gdicmn.h>
#include "UtilInt.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
}

namespace test {

/// Trim the given clip on the left side
void TrimLeft(model::IClipPtr clip, pixel length, bool shift = true, bool endtrim = true);

/// Trim the given clip on the right side
void TrimRight(model::IClipPtr clip, pixel length, bool shift = true, bool endtrim = true);

void BeginTrim(wxPoint from, bool shift);

void EndTrim(bool shift = true);

/// Do a trim between the two points (press, move, release). This basically does the same as 'Drag' but faster. The Drag
/// method does the move in several (10) steps. This method simply moves to the begin point, presses the mouse, moves
/// to the end point (without intermediate points) and releases the button.
/// \param from starting position to move to initially
/// \param to final position to drag to
/// \param shift hold down shift after pressing the mouse button
void Trim(wxPoint from, wxPoint to, bool shift = false, bool endtrim = true);

/// \see Trim
/// Do a shift trim
void ShiftTrim(wxPoint from, wxPoint to);

} // namespace

#endif // HELPER_TRIM_H