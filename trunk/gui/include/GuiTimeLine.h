#ifndef GUI_TIME_LINE_H
#define GUI_TIME_LINE_H

#include <wx/scrolwin.h>
#include <wx/dnd.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "GuiTimeLineDragImage.h"
#include "GuiDataObject.h"
#include "ModelPtr.h"
#include "GuiPlayer.h"
#include "GuiPtr.h"
#include "TrackUpdateEvent.h"
#include "GuiTimeLineMouseState.h"

class ProjectEventAddAsset;
class ProjectEventDeleteAsset;
class ProjectEventRenameAsset;

class GuiTimeLine
:   public wxScrolledWindow
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /** The '0' pointer is used for recovery. */
    GuiTimeLine(model::SequencePtr sequence = model::SequencePtr());

    /**
     * Two step construction. First the constructor (in combination with serialize)
     * sets all relevant  members. Second, this method initializes all GUI stuff 
     * including the bitmap. 
     */
    void init(wxWindow *parent);

    virtual ~GuiTimeLine();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnProjectAssetAdded( ProjectEventAddAsset &event );
    void OnProjectAssetDeleted( ProjectEventDeleteAsset &event );
    void OnProjectAssetRenamed( ProjectEventRenameAsset &event );

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnPaint( wxPaintEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // DRAWING EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnTrackUpdated( TrackUpdateEvent& event );
    void beginDrag(wxPoint position);
    void moveDrag(wxPoint position);
    void endDrag(wxPoint position);


    wxBitmap getDragBitmap(wxPoint& hostspot);// const;
    void updateBitmap();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr getSequence() const;
    int getWidth() const;

    //////////////////////////////////////////////////////////////////////////
    // CURSOR
    //////////////////////////////////////////////////////////////////////////

    void moveCursorOnUser(int position);
    void setCursorPosition(long position);
    void moveCursorOnPlayback(long pts);

private:
    GuiTimeLineZoomPtr mZoom;
    PlayerPtr mPlayer;
    wxBitmap mBitmap;
    long mCursorPosition;
    double mPlaybackTime;
    long mWidth;
    long mHeight;

    /** Y-position of audio-video divider */
    int mDividerPosition;
    
    wxPoint         mDragStartPosition;
    GuiTimeLineDragImage*    m_dragImage;

    mousestate::Machine mMouseState;

    model::SequencePtr mSequence;

    GuiTimeLineTracks mVideoTracks;
    GuiTimeLineTracks mAudioTracks;

    wxPoint mOrigin;

    //////////////////////////////////////////////////////////////////////////
    // FROM COORDINATES TO OBJECTS
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLineClipPtr GuiTimeLine::findClip(wxPoint p) const;
    GuiTimeLineTrackPtr findTrack(int yposition) const;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateSize();
    void DetermineWidth();
    void DetermineHeight(); 

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive> 
    void serialize(Archive & ar, const unsigned int version);
};

BOOST_CLASS_VERSION(GuiTimeLine, 1)
BOOST_CLASS_EXPORT(GuiTimeLine)

#endif // GUI_TIME_LINE_H