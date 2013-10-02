// Copyright 2013 Eric Raijmakers.
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

#include "EmptyFrame.h"

#include "UtilInitAvcodec.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyFrame::EmptyFrame(wxSize size)
:   VideoFrame(size)
,   mInitialized(false)
{
}

EmptyFrame::~EmptyFrame()
{
    // Deallocation handled by base class
}

//////////////////////////////////////////////////////////////////////////
// DATA ACCESS
//////////////////////////////////////////////////////////////////////////

DataPointer EmptyFrame::getData()
{
    if (!mInitialized)
    {
        PixelFormat format = PIX_FMT_RGB24;
        mBufferSize = avpicture_get_size(format, getSize().GetWidth(), getSize().GetHeight());
        mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));
        memset(mBuffer,0,mBufferSize);

        mFrame = avcodec_alloc_frame();

        // Assign appropriate parts of buffer to image planes in mFrame
        avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, format,getSize().GetWidth(), getSize().GetHeight());
        mInitialized = true;
    }
    return VideoFrame::getData();
}

} // namespace