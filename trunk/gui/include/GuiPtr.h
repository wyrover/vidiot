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

//////////////////////////////////////////////////////////////////////////

class GuiTimeLine;
typedef GuiTimeLine* GuiTimeLinePtr; // Lifetime managed by wxwidgets

//////////////////////////////////////////////////////////////////////////

class GuiTimeLineTrack;
typedef GuiTimeLineTrack* GuiTimeLineTrackPtr;

typedef std::list<GuiTimeLineTrackPtr> GuiTimeLineTracks;

//////////////////////////////////////////////////////////////////////////

class GuiTimeLineClip;
typedef GuiTimeLineClip* GuiTimeLineClipPtr;

//////////////////////////////////////////////////////////////////////////

class SelectIntervals;
typedef boost::shared_ptr<SelectIntervals> IntervalsPtr;

//////////////////////////////////////////////////////////////////////////

}} // namespace

#endif // GUI_PTR_H
