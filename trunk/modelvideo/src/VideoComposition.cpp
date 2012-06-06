#include "VideoFrameComposition.h"

#include <wx/dcmemory.h>
#include <wx/dcgraph.h>
#include <wx/bitmap.h>
#include <wx/graphics.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Convert.h"
#include "EmptyFrame.h"
#include "Layout.h"
#include "Properties.h"
#include "UtilList.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrameComposition::VideoFrameComposition(const VideoParameters& parameters)
    :   mFrames()
    ,   mParameters(parameters)
{
    VAR_DEBUG(this);
}

VideoFrameComposition::VideoFrameComposition(const VideoFrameComposition& other)
    :   mFrames(other.mFrames)
    ,   mParameters(other.mParameters)
{
}

VideoFrameComposition::~VideoFrameComposition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// COMPOSITION
//////////////////////////////////////////////////////////////////////////

void VideoFrameComposition::add(VideoFramePtr frame)
{
    if (frame && !frame->isA<EmptyFrame>())
    {
        // Skip empty frames.
        mFrames.push_back(frame);
    }
}

void VideoFrameComposition::replace(VideoFramePtr oldFrame, VideoFramePtr newFrame)
{
    UtilList<model::VideoFramePtr>(mFrames).replace(oldFrame,newFrame);
}

VideoFramePtr VideoFrameComposition::generate()
{
    if (mFrames.empty())
    {
        return EmptyFramePtr();
    }

    wxSize outputsize = Properties::get()->getVideoSize();
    double scaleToBoundingBox(0);
    wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, mParameters.getBoundingBox(), scaleToBoundingBox);
    ASSERT_NONZERO(scaleToBoundingBox);

    wxImagePtr compositeImage(boost::make_shared<wxImage>(requiredOutputSize));
    wxGraphicsContext* gc = wxGraphicsContext::Create(*compositeImage);

    BOOST_FOREACH( VideoFramePtr frame, mFrames )
    {
        wxImagePtr image = frame->getImage();
        if (image) // image may be '0' due to clipping/moving
        {
            image->InitAlpha();
            unsigned char* alpha = image->GetAlpha();
            memset(alpha,frame->getOpacity(),image->GetWidth() * image->GetHeight());
            gc->DrawBitmap(gc->GetRenderer()->CreateBitmapFromImage(*image),frame->getPosition().x,frame->getPosition().y,image->GetWidth(),image->GetHeight());
        }
    }

    if (mParameters.getDrawBoundingBox())
    {
        gc->SetPen(gui::Layout::sPreviewBoundingBoxPen);
        gc->SetBrush(*wxTRANSPARENT_BRUSH);
        gc->DrawRectangle( 1, 1, requiredOutputSize.GetWidth() - 1, requiredOutputSize.GetHeight() - 1);
    }

    delete gc;
    VideoFramePtr result = boost::make_shared<VideoFrame>(compositeImage,0);
    return result;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoParameters VideoFrameComposition::getParameters() const
{
    return mParameters;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoFrameComposition& obj )
{
    os << &obj << '|' << obj.mFrames << '|' << obj.mBoundingBox;
    return os;
}

} //namespace