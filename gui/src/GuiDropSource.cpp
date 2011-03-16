#include "GuiDropSource.h"

#include <wx/frame.h>
#include <wx/dcclient.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////
// todo obsolete?
GuiDropSource::GuiDropSource(wxWindow* win, model::ProjectViewPtrs assets)
:   wxDropSource(win,wxDROP_ICON(dnd_copy),wxDROP_ICON(dnd_move),wxDROP_ICON(dnd_none))
//,   mDataObject(assets)
,   mAssets(assets)
,   mWin(win)
,   mHint(0)
{
    SetData(mDataObject);
}

GuiDropSource::~GuiDropSource() 
{
    delete mHint;
}

//////////////////////////////////////////////////////////////////////////
// FROM WXDROPSOURCE
//////////////////////////////////////////////////////////////////////////

wxDragResult GuiDropSource::DoDragDrop(int flags)
{
    //wxDrag_CopyOnly    = 0, // allow only copying
    //    wxDrag_AllowMove   = 1, // allow moving (copying is always allowed)
    //    wxDrag_DefaultMove

    return wxDropSource::DoDragDrop(wxDrag_AllowMove);
}
bool GuiDropSource::GiveFeedback(wxDragResult effect)
{
    return false;
    //wxPoint pos = wxGetMousePosition();

    //if (!mHint)
    //{
    //    //int liney = m_win->GetLineStart( m_row );
    //    //int linex = 0;
    //    //m_win->GetOwner()->CalcUnscrolledPosition( 0, liney, NULL, &liney );
    //    //m_win->ClientToScreen( &linex, &liney );
    //    //m_dist_x = pos.x - linex;
    //    //m_dist_y = pos.y - liney;

    //    wxBitmap ib( mAssets.size() * 50, 20 );
    //    wxMemoryDC dc( ib );
    //    //dc.SetFont( /*GetFont*/() );
    //    dc.SetPen( *wxBLACK_PEN );
    //    dc.SetBrush( *wxWHITE_BRUSH );
    //    dc.DrawRectangle( 0,0,mAssets.size() * 50, 20 );

    //    //m_dist_x -= indent;


    //    mHint = new wxFrame( mWin->GetParent(), wxID_ANY, wxEmptyString,
    //        //wxPoint(pos.x - m_dist_x, pos.y + 5 ),
    //        wxPoint(pos.x, pos.y + 5 ),
    //        ib.GetSize(),
    //        wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxNO_BORDER );
    //    //new wxBitmapCanvas( mHint, ib, ib.GetSize() );
    //    mHint->Show();
    //}
    //else
    //{
    //    //m_hint->Move( pos.x - m_dist_x, pos.y + 5  );
    //    mHint->Move( pos.x, pos.y + 5  );
    //    mHint->SetTransparent( 128 );
    //}

    //return false;


}

} // namespace
