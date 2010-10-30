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
typedef boost::shared_ptr<GuiTimeLineTrack> GuiTimeLineTrackPtr;

typedef std::list<GuiTimeLineTrackPtr> GuiTimeLineTracks;

//////////////////////////////////////////////////////////////////////////

class GuiTimeLineClip;
typedef boost::shared_ptr<GuiTimeLineClip> GuiTimeLineClipPtr;

typedef std::list<GuiTimeLineClipPtr> GuiTimeLineClips;

typedef boost::tuple<GuiTimeLineClipPtr,int> GuiTimeLineClipWithOffset;

//////////////////////////////////////////////////////////////////////////

class GuiTimeLineZoom;
typedef boost::shared_ptr<GuiTimeLineZoom> GuiTimeLineZoomPtr;

//////////////////////////////////////////////////////////////////////////

class SelectIntervals;
typedef boost::shared_ptr<SelectIntervals> IntervalsPtr;

//////////////////////////////////////////////////////////////////////////

typedef std::map< model::TrackPtr, GuiTimeLineTrackPtr > TrackMap;
typedef std::map< model::ClipPtr, GuiTimeLineClipPtr > ClipMap;

}} // namespace

#endif // GUI_PTR_H
