#ifndef GUI_PTR_H
#define GUI_PTR_H

// This file contains all forward declarations of gui shared pointers.

#include <list>
#include <boost/shared_ptr.hpp>

namespace gui {
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

//////////////////////////////////////////////////////////////////////////

class GuiTimeLineZoom;
typedef boost::shared_ptr<GuiTimeLineZoom> GuiTimeLineZoomPtr;

//////////////////////////////////////////////////////////////////////////

}} // namespace

#endif // GUI_PTR_H
