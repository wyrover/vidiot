// Copyright 2013-2015 Eric Raijmakers.
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
#include "Properties.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "VideoFrameLayer.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ImageFile::ImageFile()
    : VideoFile{}
    , mInputFrame{}
    , mOutputFrame{}
    , mColor{ boost::none }
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(const wxFileName& path)
    : VideoFile(path)
    , mInputFrame{}
    , mOutputFrame{}
    , mColor{ boost::none }
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(const wxColour& color)
    : VideoFile()
    , mInputFrame{}
    , mOutputFrame{}
    , mColor{ color }
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(const ImageFile& other)
    : VideoFile(other)
    , mInputFrame{}
    , mOutputFrame{}
    , mColor{ other.mColor }
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
        if (mColor)
        {
            wxImagePtr image = boost::make_shared<wxImage>(getSize());
            image->SetRGB(wxRect{ wxPoint{ 0, 0 }, getSize() }, mColor->Red(), mColor->Green(), mColor->Blue());
            mInputFrame = boost::make_shared<VideoFrame>(parameters, boost::make_shared<VideoFrameLayer>(image));
        }
        else
        {
            mInputFrame = VideoFile::getNextVideo(VideoCompositionParameters().setBoundingBox(getSize()).setPts(0));
        }
        ASSERT(mInputFrame);
    }

    if (!mOutputFrame || parameters.getBoundingBox() != mOutputFrame->getParameters().getBoundingBox())
    {
        wxImagePtr outputImage = boost::make_shared<wxImage>(mInputFrame->getImage()->Copy());
        outputImage->Rescale(parameters.getBoundingBox().x, parameters.getBoundingBox().y, wxIMAGE_QUALITY_HIGH);
        mOutputFrame = boost::make_shared<VideoFrame>(parameters,boost::make_shared<VideoFrameLayer>(outputImage));
    }
    // Frame must be cloned, frame repeating is not supported. If a frame is to be output multiple
    // times, avoid pts calculation problems by making multiple unique frames.
    //
    // Furthermore, note that the returned frame may have already been queued somewhere (VideoDisplay, for example).
    // Changing the frame and returning that once more might thus change that previous frame also!
    return make_cloned<VideoFrame>(mOutputFrame);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize ImageFile::getSize()
{
    if (mColor)
    {
        return Properties::get().getVideoSize();
    }
    return VideoFile::getSize(); // Todo make class that inherits from image file instead?
}

bool ImageFile::canBeOpened()
{
    if (mColor)
    {
        return true;
    }
    return VideoFile::canBeOpened();
}


//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ImageFile& obj)
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
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoFile);
        if (version >= 2)
        {
            ar & BOOST_SERIALIZATION_NVP(mColor);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void ImageFile::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ImageFile::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::ImageFile)