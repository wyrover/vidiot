#include "Drag.h"

#include <wx/pen.h>
#include <wx/tooltip.h>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/assign/list_of.hpp>
#include "Timeline.h"
#include "MousePointer.h"
#include "VideoFile.h"
#include "AudioFile.h"
#include "PositionInfo.h"
#include "Layout.h"
#include "VideoClip.h"
#include "AudioClip.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "File.h"
#include "State.h"
#include "TrackView.h"
#include "ProjectView.h"
#include "UtilLogWxwidgets.h"
#include "UtilList.h"
#include "Track.h"
#include "Sequence.h"
#include "Selection.h"
#include "SequenceView.h"
#include "ViewMap.h"
#include "Zoom.h"
#include "GuiDataObject.h"
#include "UtilLogStl.h"
#include "Divider.h"
#include "Clip.h"
#include "ClipView.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drag::Drag(Timeline* timeline)
    :   Part(timeline)
    ,   wxDropTarget(new GuiDataObject())
    ,   mIsInsideDrag(true)
    ,   mHotspot(0,0)
    ,   mPosition(0,0)
    ,   mBitmapOffset(0,0)
    ,   mBitmap()
    ,   mActive(false)
    ,   mDraggedTrack()
    ,   mDropTrack()
    ,   mVideo(timeline, true)
    ,   mAudio(timeline, false)
{
    VAR_DEBUG(this);
    getTimeline().SetDropTarget(this);// Uncommented, since the destruction will then be done by wxwidgets
}

Drag::~Drag()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// START/STOP
//////////////////////////////////////////////////////////////////////////

void Drag::start(wxPoint hotspot, bool isInsideDrag)
{
    PointerPositionInfo info = getMousePointer().getInfo(hotspot);

    reset();
    mActive = true;
    mIsInsideDrag = isInsideDrag;
    mHotspot = hotspot;
    mPosition = hotspot;
    mBitmapOffset = wxPoint(0,0);
    mDropTrack = info.track;

    if (!mIsInsideDrag)
    {
        makeTracksFromProjectView();
        mDraggedTrack = mVideo.getTempTrack();
        mHotspot.x = getZoom().ptsToPixels(mVideo.getTempTrack()->getLength() / 2);

        // When dragging new clips into the timeline, the clips also need to be removed first.
        // This ensures that any used View classes are destroyed. Otherwise, there remain multiple
        // Views for a clip.
        UtilList<model::ClipPtr>(mDraggedClips).addElements(mVideo.getTempTrack()->getClips(),model::ClipPtr());
        UtilList<model::ClipPtr>(mDraggedClips).addElements(mAudio.getTempTrack()->getClips(),model::ClipPtr());
    }
    else
    {
        mDraggedTrack = info.track;
        UtilList<model::ClipPtr>(mDraggedClips).addElements(getSelection().getClips());
        invalidateDraggedClips(); // Hide dragged clips: Not necessary when dropping new assets into the timeline, since these do not have to be 'hidden' from the timeline
    }
    VAR_DEBUG(*this);
    ASSERT(mDraggedTrack);

    determinePossibleSnapPoints(); // Required initialized mDraggedClips
    mBitmap = getDragBitmap();
    move(hotspot, false);
}

void Drag::move(wxPoint position, bool altPressed)
{
    VAR_DEBUG(*this);
    wxRegion redrawRegion;

    redrawRegion.Union(wxRect(mBitmapOffset + mPosition - mHotspot, mBitmap.GetSize())); // Redraw the old area (moved 'out' of this area)

    if (position.x - mHotspot.x + mBitmapOffset.x < 0)
    {
        position.x = mHotspot.x - mBitmapOffset.x; // Can't move 'beyond 0'
    }

    PointerPositionInfo info = getMousePointer().getInfo(position);

    if (altPressed)
    {
        // As long as ALT is down, the dragged image stays the same, but the hotspot is moved
        mHotspot -=  mPosition - position;
        updateDraggedTrack(info.track);
    }
    else if (!info.track || info.track == mDropTrack)
    {
        // Mouse is moved within the current track.
        // No changes in mDraggedTrack or mDroppedTrack required
        //
        // Move the cursor without moving the dragged object (note: vertical only!)
        mHotspot.y -= mPosition.y - position.y;
    }
    else
    {
        // The pointer is moved to another track.
        if (info.track->isA<model::VideoTrack>() == mDraggedTrack->isA<model::VideoTrack>())
        {
            // The pointer moved between video tracks or between audio tracks.
            updateOffset(info.track);
        }
        else
        {
            // Pointer moved from video to audio. The 'dragged track' must be updated.
            // The offsets are not changed. These are only changed when moving a track
            // of type x to another track of type x.
            updateDraggedTrack(info.track);
        }
        mHotspot.y = position.y;
        mBitmap = getDragBitmap();
    }

    mDropTrack = info.track;
    mPosition = position;
    redrawRegion.Union(wxRect(mBitmapOffset + mPosition - mHotspot, mBitmap.GetSize())); // Redraw the new area (moved 'into' this area)

    determineSnapOffset();
    getSequenceView().invalidateBitmap();

    wxRegionIterator it(redrawRegion);
    while (it)
    {
        getTimeline().RefreshRect(it.GetRect(), false);
        it++;
    }

    VAR_DEBUG(*this);
}

void Drag::drop()
{
    command::ExecuteDrop::Drops drops;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        drops.splice(drops.end(), getDrops(track));
    }
    getTimeline().Submit(new command::ExecuteDrop(getTimeline(),mDraggedClips,drops));
}

void Drag::stop()
{
    VAR_DEBUG(*this);
    mActive = false;            // Ensure that moved clips are not blanked out anymore. See ClipView::draw().
    invalidateDraggedClips();   // Ensure that moved clips are not blanked out anymore. See ClipView::draw().
    reset();
    getTimeline().Refresh();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool Drag::isActive() const
{
    return mActive;
}

bool Drag::contains(model::ClipPtr clip) const
{
    return UtilList<model::ClipPtr>(static_cast<const model::Clips>(mDraggedClips)).hasElement(clip);
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

wxBitmap Drag::getDragBitmap() //const
{
    VAR_DEBUG(*this);
    int w = getTimeline().requiredWidth();
    int h = getTimeline().requiredHeight();

    wxBitmap temp(w,h); // Create a bitmap equal in size to the entire virtual area (for simpler drawing code)
    wxBitmap mask(w,h,1);

    wxMemoryDC dc(temp); // Must go out of scope to be able to use temp.data below
    wxMemoryDC dcMask(mask);

    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0,0,w,h);

    dcMask.SetBackground(*wxBLACK_BRUSH);
    dcMask.Clear();
    dcMask.SetPen(*wxWHITE_PEN);
    dcMask.SetBrush(*wxWHITE_BRUSH);

    // Draw video tracks
    wxPoint position(0,getDivider().getVideoPosition());
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        position.y += Layout::sTrackDividerHeight;
        model::TrackPtr draggedTrack = trackOnTopOf(track);
        if (draggedTrack)
        {
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        position.y += track->getHeight();
    }

    // Draw audio tracks
    position.y = getDivider().getAudioPosition();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        model::TrackPtr draggedTrack = trackOnTopOf(track);
        if (draggedTrack)
        {
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        position.y += track->getHeight() + Layout::sTrackDividerHeight;
    }

    mBitmapOffset.x = std::max(dcMask.MinX(),0);
    mBitmapOffset.y = std::max(dcMask.MinY(),0);

    int size_x = dcMask.MaxX() - mBitmapOffset.x;
    int size_y = dcMask.MaxY() - mBitmapOffset.y;

    dc.SelectObject(wxNullBitmap);
    dcMask.SelectObject(wxNullBitmap);

    temp.SetMask(new wxMask(mask));

    VAR_DEBUG(mBitmapOffset)(size_x)(size_y);
    ASSERT(size_x > 0 && size_y > 0)(size_x)(size_y);
    return temp.GetSubBitmap(wxRect(mBitmapOffset.x,mBitmapOffset.y,size_x,size_y));
}

void Drag::draw(wxDC& dc) const
{
    if (!mActive)
    {
        return;
    }
    dc.DrawBitmap(mBitmap,mBitmapOffset + getDraggedDistance() + wxPoint(getZoom().ptsToPixels(mSnapOffset),0),true);
    dc.SetPen(Layout::sSnapPen);
    dc.SetBrush(Layout::sSnapBrush);
    BOOST_FOREACH( pts snap, mSnaps )
    {
        dc.DrawLine(getZoom().ptsToPixels(snap),0,getZoom().ptsToPixels(snap),dc.GetSize().GetHeight());
    }
}

//////////////////////////////////////////////////////////////////////////
// FROM WXDROPTARGET
//////////////////////////////////////////////////////////////////////////

wxDragResult Drag::OnEnter (wxCoord x, wxCoord y, wxDragResult def)
{
    ProjectViewDropSource::current().setFeedback(false);
    getStateMachine().process_event(state::EvDragEnter(x,y));
    return wxDragMove;
}

wxDragResult Drag::OnDragOver (wxCoord x, wxCoord y, wxDragResult def)
{
    getStateMachine().process_event(state::EvDragMove(x,y));
    return wxDragMove;
}

bool Drag::OnDrop (wxCoord x, wxCoord y)
{
    getStateMachine().process_event(state::EvDragDrop(x,y));
    return true;
}

void Drag::OnLeave()
{
    ProjectViewDropSource::current().setFeedback(true);
    getStateMachine().process_event(state::EvDragEnd(0,0));
}

//////////////////////////////////////////////////////////////////////////
// DRAGINFO
//////////////////////////////////////////////////////////////////////////

/// Dummy class to be able to create views for tracks and clips in case of adding them from the project view.
/// This is a 'top' view class that ignores all events.
class DummyView : public View
{
public:
    DummyView(Timeline* timeline);
    virtual ~DummyView();
    virtual pixel requiredWidth() const;
    virtual pixel requiredHeight() const;
    virtual void draw(wxBitmap& bitmap) const;
};

DummyView::DummyView(Timeline* timeline) 
:   View(timeline) 
{
}

DummyView::~DummyView()
{
}

pixel DummyView::requiredWidth() const 
{ 
    return getTimeline().requiredWidth(); 
}

pixel DummyView::requiredHeight() const 
{ 
    return getTimeline().requiredHeight(); 
}

void DummyView::draw(wxBitmap& bitmap) const 
{ 
    FATAL(""); 
}

Drag::DragInfo::DragInfo(Timeline* timeline, bool isVideo)
:   Part(timeline)
,   mIsVideo(isVideo)
,   mOffset(0)
,   mMinOffset(0)
,   mMaxOffset(0)
,   mTempTrack()
,   mView(new DummyView(timeline))
{
}

Drag::DragInfo::~DragInfo()
{
    delete mView;
}

void Drag::DragInfo::reset()
{
    mOffset = 0;
    mMinOffset = 0;
    mMaxOffset = 0;

    if (mTempTrack)
    {
        // Remove previous track
        delete getViewMap().getView(mTempTrack);
    }
    mTempTrack.reset();

    // Determine boundaries for 'inside' drags
    std::set<model::TrackPtr> selectedTracks;
    BOOST_FOREACH( model::ClipPtr clip, getSelection().getClips() )
    {
        model::TrackPtr track = clip->getTrack();
        if (track->isA<model::VideoTrack>() == mIsVideo)
        {
            mMinOffset = std::min(mMinOffset, track->getIndex() * -1);
            mMaxOffset = std::max(mMaxOffset, nTracks() - track->getIndex());
        }
    }
}

void Drag::DragInfo::updateOffset(int indexOfTrackInTimeline, int indexOfDraggedTrack)
{
    if (!mTempTrack)
    {
        // 'inside' drag
        mOffset = 
            std::min(mMaxOffset,
            std::max(mMinOffset, 
            indexOfTrackInTimeline - indexOfDraggedTrack));
    }
    else
    {
        // 'outside' drag
        mOffset = indexOfTrackInTimeline;
    }
}


model::TrackPtr Drag::DragInfo::getTrack(int index)
{
    return  mIsVideo ? getSequence()->getVideoTrack(index) : getSequence()->getAudioTrack(index);
}

model::TrackPtr Drag::DragInfo::getTempTrack()
{
    return mTempTrack;
}

void Drag::DragInfo::setTempTrack(model::TrackPtr track)
{
    if (mTempTrack)
    {
        // Remove previous track
        delete getViewMap().getView(mTempTrack);
    }
    mTempTrack = track;
    new TrackView(mTempTrack, mView);
}

model::TrackPtr Drag::DragInfo::trackOnTopOf(model::TrackPtr track)
{
    VAR_DEBUG(track);
    model::TrackPtr draggedTrack;
    if (!mTempTrack)
    {
        // 'inside' drag
        draggedTrack = getTrack(track->getIndex() - mOffset);
    }
    else
    {
        // 'outside' drag
        if (mOffset == track->getIndex())
        {
            draggedTrack = mTempTrack;
        }
    }
    VAR_DEBUG(draggedTrack);
    return draggedTrack;
}

int Drag::DragInfo::nTracks()
{
    return mIsVideo ? getSequence()->getVideoTracks().size() : getSequence()->getAudioTracks().size();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Drag::reset()
{
    mDraggedClips.clear();
    mHotspot = wxPoint(0,0);
    mPosition = wxPoint(0,0);
    mBitmapOffset = wxPoint(0,0);
    mSnapPoints.clear();
    mDragPoints.clear();
    mSnapOffset = 0;
    mSnaps.clear();
    mVideo.reset();
    mAudio.reset();
    mDraggedTrack.reset();
    mDropTrack.reset();
}

void Drag::makeTracksFromProjectView()
{
    std::list<model::ProjectViewPtr> draggedAssets = ProjectViewDropSource::current().getData().getAssets();
    // todo refactor into method to be reused in createsequencecommand...

    model::VideoTrackPtr videoTrack = boost::make_shared<model::VideoTrack>();
    model::AudioTrackPtr audioTrack = boost::make_shared<model::AudioTrack>();

    BOOST_FOREACH( model::ProjectViewPtr asset, draggedAssets )
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(asset);
        if (file)
        {
            VAR_DEBUG(file);
            model::VideoFilePtr videoFile = boost::make_shared<model::VideoFile>(file->getPath());
            model::AudioFilePtr audioFile = boost::make_shared<model::AudioFile>(file->getPath());
            // todo hasvideo and has audio. If not, use emptyclip in other track
            model::VideoClipPtr videoClip = boost::make_shared<model::VideoClip>(videoFile);
            model::AudioClipPtr audioClip = boost::make_shared<model::AudioClip>(audioFile);
            videoClip->setLink(audioClip);
            audioClip->setLink(videoClip);
            videoTrack->addClips(boost::assign::list_of(videoClip));
            audioTrack->addClips(boost::assign::list_of(audioClip));
        }
    }
    mVideo.setTempTrack(videoTrack);
    mAudio.setTempTrack(audioTrack);
}

model::TrackPtr Drag::trackOnTopOf(model::TrackPtr track)
{
    return getAssociatedInfo(track).trackOnTopOf(track);
}

Drag::DragInfo& Drag::getAssociatedInfo(model::TrackPtr track)
{
    return track->isA<model::VideoTrack>() ? mVideo : mAudio;
}

void Drag::updateOffset(model::TrackPtr trackUnderPointer)
{
    getAssociatedInfo(trackUnderPointer).updateOffset(trackUnderPointer->getIndex(), mDraggedTrack->getIndex());
}

void Drag::updateDraggedTrack(model::TrackPtr track)
{
    if (track)
    {
        model::TrackPtr newDraggedTrack = trackOnTopOf(track);
        if (newDraggedTrack)
        {
            mDraggedTrack = newDraggedTrack;
        }
    }
}

void Drag::invalidateDraggedClips()
{
    BOOST_FOREACH( model::ClipPtr clip, mDraggedClips )
    {
        getViewMap().getView(clip)->invalidateBitmap();
    }
}

wxPoint Drag::getDraggedDistance() const
{
    return mPosition - mHotspot;
}

pts Drag::getDraggedPts() const
{
    return getZoom().pixelsToPts(getDraggedDistance().x);
}

void Drag::determineSnapOffset()
{
    pts ptsoffset = getDraggedPts();
    pts ptsmouse = getZoom().pixelsToPts(mPosition.x);

    // Find nearest snap match
    pts minDiff = Layout::sSnapDistance + 1; // To ensure that the first found point will change this value
    pts snapPoint = -1;
    pts snapOffset = 0;
    std::list<pts>::const_iterator itTimeline = mSnapPoints.begin();
    std::list<pts>::const_iterator itDrag = mDragPoints.begin();
    while ( itTimeline != mSnapPoints.end() && itDrag != mDragPoints.end() )
    {
        pts pts_timeline = *itTimeline;
        pts pts_drag = *itDrag + ptsoffset;

        pts diff = abs(pts_drag - pts_timeline);
        if (diff <= Layout::sSnapDistance)
        {
            // This snap point is closer than the currently stored snap point, or it is equally
            // close, but is closer to the mouse pointer.
            if ((diff < minDiff) || 
                ((diff == minDiff) && (abs(pts_drag - ptsmouse) < abs(snapPoint - ptsmouse))))
            {
                minDiff = diff;
                snapPoint = pts_timeline;
                snapOffset = pts_timeline - pts_drag;
            }
        }
        if (pts_timeline <= pts_drag)
        {
            ++itTimeline;
        }
        if (pts_timeline >= pts_drag)
        {
            ++itDrag;
        }
    }

    mSnapOffset = snapOffset;

    // Now determine all 'snaps' (positions where dragged cuts and timeline cuts are aligned)
    mSnaps.clear();
    itTimeline = mSnapPoints.begin();
    itDrag = mDragPoints.begin();
    while ( itTimeline != mSnapPoints.end() && itDrag != mDragPoints.end() )
    {
        pts pts_timeline = *itTimeline;
        pts pts_drag = *itDrag + ptsoffset + mSnapOffset;
        if (pts_timeline == pts_drag)
        {
            mSnaps.push_back(pts_timeline);
        }
        if (pts_timeline <= pts_drag)
        {
            ++itTimeline;
        }
        if (pts_timeline >= pts_drag)
        {
            ++itDrag;
        }
    }
}

void Drag::determinePossibleSnapPoints()
{
    mSnapPoints.clear();
    mDragPoints.clear();

    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::ClipPtr clip, track->getClips() )
        {
            if (!contains(clip))
            {
                mSnapPoints.push_back(clip->getLeftPts());
                mSnapPoints.push_back(clip->getRightPts());
            }
        }
    }

    BOOST_FOREACH( model::ClipPtr clip, mDraggedClips )
    {
        if (contains(clip))
        {
            mDragPoints.push_back(clip->getLeftPts());
            mDragPoints.push_back(clip->getRightPts());
        }
    }

    mSnapPoints.sort();
    mSnapPoints.unique();
    mDragPoints.sort();
    mDragPoints.unique();
}

command::ExecuteDrop::Drops Drag::getDrops(model::TrackPtr track)
{
    command::ExecuteDrop::Drops drops;
    model::TrackPtr draggedTrack = trackOnTopOf(track);
    if (draggedTrack)
    {
        pts position = 0;
        command::ExecuteDrop::Drop pi;
        pi.position = -1;
        pi.track = track;
        bool inregion = false;

        BOOST_FOREACH( model::ClipPtr clip, draggedTrack->getClips() )
        {
            if (!inregion && contains(clip))
            {
                inregion = true;
                pi.position = position + mSnapOffset + getDraggedPts();
            }
            if (inregion && !contains(clip))
            {
                inregion = false;
                drops.push_back(pi);
                pi.position = -1; // Prepare for new region
                pi.clips.clear(); // Prepare for new region
            }
            if (inregion)
            {
                pi.clips.push_back(clip);
            }
            position += clip->getLength();
        }
        if (inregion)
        {
            drops.push_back(pi); // Insertion at end
        }
    }
    return drops;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const Drag& obj)
{
    os  << &obj                 << '|'
        << obj.mSnap            << '|'
        << obj.mActive          << '|'
        << obj.mHotspot         << '|'
        << obj.mPosition        << '|' 
        << obj.mVideo           << '|'
        << obj.mAudio           << '|'
        << obj.mDraggedTrack    << '|'
        << obj.mDropTrack;
    return os;
}

std::ostream& operator<< (std::ostream& os, const Drag::DragInfo& obj)
{
    os  << obj.mIsVideo     << '|'
        << obj.mOffset      << '|'
        << obj.mMinOffset   << '|'
        << obj.mMaxOffset   << '|'
        << obj.mTempTrack;
    return os;
}

}} // namespace

