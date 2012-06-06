#include "EditDisplay.h"

#include <wx/dcclient.h>
#include "UtilLog.h"
#include "Layout.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

EditDisplay::EditDisplay(wxWindow *parent)
:   wxControl(parent, wxID_ANY)
,	mWidth(0)
,	mHeight(0)
,   mCurrentBitmap()
{
    GetClientSize(&mWidth,&mHeight);
    VAR_DEBUG(this)(mWidth)(mHeight);

    Bind(wxEVT_PAINT,               &EditDisplay::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &EditDisplay::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &EditDisplay::OnSize,               this);

	LOG_INFO;
}

EditDisplay::~EditDisplay()
{
    VAR_DEBUG(this);
    Unbind(wxEVT_PAINT,               &EditDisplay::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &EditDisplay::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &EditDisplay::OnSize,               this);
}

//////////////////////////////////////////////////////////////////////////
// SHOW AN EDIT ACTION
//////////////////////////////////////////////////////////////////////////

void EditDisplay::show(boost::shared_ptr<wxBitmap> bitmap)
{
    mCurrentBitmap = bitmap;
    Refresh(false);
    Update(); // Immediate feedback, do not wait until next onPaint
}

//////////////////////////////////////////////////////////////////////////
// GET & SET
//////////////////////////////////////////////////////////////////////////

wxSize EditDisplay::getSize() const
{
    return wxSize(mWidth,mHeight);
}

//////////////////////////////////////////////////////////////////////////
// GUI METHODS
//////////////////////////////////////////////////////////////////////////

inline void EditDisplay::OnEraseBackground(wxEraseEvent& event)
{
    // do nothing
}

void EditDisplay::OnSize(wxSizeEvent& event)
{
    int w = mWidth;
    int h = mHeight;
    GetClientSize(&w,&h);
    if (mWidth != w || mHeight != h)
    {
        mWidth = w;
        mHeight = h;
        VAR_INFO(mWidth)(mHeight);

        Refresh();
    }
}

void EditDisplay::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this); // Omit this, and suffer the performance consequences ;-)

    dc.SetPen(Layout::sPreviewBackgroundPen);
    dc.SetBrush(Layout::sPreviewBackgroundBrush);
    if (mCurrentBitmap)
    {
        if (mCurrentBitmap->GetWidth() < mWidth)
        {
            dc.DrawRectangle( mCurrentBitmap->GetWidth(), 0, mWidth - mCurrentBitmap->GetWidth(), mCurrentBitmap->GetHeight());
        }
        if (mCurrentBitmap->GetHeight() < mHeight)
        {
            dc.DrawRectangle( 0, mCurrentBitmap->GetHeight(), mWidth, mHeight - mCurrentBitmap->GetHeight());
        }
        dc.DrawBitmap(*mCurrentBitmap,wxPoint(0,0));
    }
    else
    {
        dc.DrawRectangle( 0, 0, mWidth, mHeight);
    }
}

} // namespace