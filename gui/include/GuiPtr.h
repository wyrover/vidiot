#ifndef GUI_PTR_H
#define GUI_PTR_H

// This file contains all forward declarations of gui shared pointers.

#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include "ModelPtr.h"

namespace gui {

class GuiPlayer;
typedef boost::shared_ptr<GuiPlayer> PlayerPtr;

namespace timeline {

class ClipView;
class Cursor;
class Drag;
class Drop;
class GuiPlayer;
class Intervals;
class Intervals;
class MenuHandler;
class MousePointer;
class Selection;
class VideoView;
class AudioView;
class Timeline;
class TrackView;
class ViewMap;
class Zoom;

namespace state {

class Machine;

}}} // namespace

#endif // GUI_PTR_H
