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

#include "ImageFile.h"

#include "Constants.h"
#include "Convert.h"
#include "UtilLog.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ImageFile::ImageFile()
:	VideoFile()
,   mInputFrame()
,   mOutputFrame()
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(wxFileName path)
:	VideoFile(path)
,   mInputFrame()
,   mOutputFrame()
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(const ImageFile& other)
:   VideoFile(other)
,   mInputFrame()
,   mOutputFrame()
{
    VAR_DEBUG(*this);
}

ImageFile* ImageFile::clone() const
{
    return new ImageFile(static_cast<const ImageFile&>(*this));
}

ImageFile::~ImageFile()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts ImageFile::getLength() const
{
    return Convert::timeToPts(Constants::sHour * 8); // 8 Hours...
}

void ImageFile::moveTo(pts position)
{
    // NOT: VideoFile::moveTo(position) NOR File::moveTo(position); - will cause crash in avcodec
}

void ImageFile::clean()
{
    VAR_DEBUG(this);
    mInputFrame.reset();
    mOutputFrame.reset();
    VideoFile::clean();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr ImageFile::getNextVideo(const VideoCompositionParameters& parameters)
{
    if (!mInputFrame)
    {
        mInputFrame = VideoFile::getNextVideo(VideoCompositionParameters().setBoundingBox(getSize()));
        ASSERT(mInputFrame);
    }

    if (!mOutputFrame || parameters.getBoundingBox() != mOutputFrame->getSize())
    {
        wxImagePtr image = mInputFrame->getImage();
        wxImagePtr outputImage = boost::make_shared<wxImage>(image->GetSize());
        int dataSize = image->GetWidth() * image->GetHeight( ) * 3; // todo update when adding alpha channel support
        memcpy(outputImage->GetData(), image->GetData(), dataSize);
        outputImage->Rescale(parameters.getBoundingBox().x, parameters.getBoundingBox().y, wxIMAGE_QUALITY_HIGH);
        mOutputFrame = boost::make_shared<VideoFrame>(outputImage, 0);
    }
    return mOutputFrame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ImageFile& obj )
{
    os << static_cast<const VideoFile&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void ImageFile::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<VideoFile>(*this);
}
template void ImageFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void ImageFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace