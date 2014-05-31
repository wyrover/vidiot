// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "EditDisplay.h"

#include "UtilLog.h"
#include "Layout.h"
#include "VideoCompositionParameters.h"

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

void EditDisplay::show(const wxBitmapPtr& bitmap)
{
    ASSERT(wxThread::IsMain()); // Access to mCurrentBitmap is not threadsafe.
    mCurrentBitmap = bitmap;
    Refresh(false);
    Update();
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

    dc.SetPen(Layout::get().PreviewBackgroundPen);
    dc.SetBrush(Layout::get().PreviewBackgroundBrush);
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
        wxMemoryDC dcBmp;
        dcBmp.SelectObject(*mCurrentBitmap);
        dc.Blit(wxPoint(0,0),mCurrentBitmap->GetSize(),&dcBmp,wxPoint(0,0));
    }
    else
    {
        dc.DrawRectangle( 0, 0, mWidth, mHeight);
    }
}

} // namespace
