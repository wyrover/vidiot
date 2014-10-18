// Copyright 2014 Eric Raijmakers.
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

#include "WximageFile.h"

#include "Constants.h"
#include "Convert.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "VideoFrameLayer.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// static 
bool WximageFile::canRead(wxFileName path)
{
    static std::set<wxString> sSupportedExts = boost::assign::list_of("png")("tiff")("tif");
    return
        sSupportedExts.find(path.GetExt()) != sSupportedExts.end() &&
        wxImage::CanRead(path.GetLongPath());
}

WximageFile::WximageFile()
: VideoFile()
{
    VAR_DEBUG(*this);
}

WximageFile::WximageFile(const wxFileName& path)
: VideoFile(path)
{
    VAR_DEBUG(*this);
}

WximageFile::WximageFile(const WximageFile& other)
: VideoFile(other)
{
    VAR_DEBUG(*this);
}

WximageFile* WximageFile::clone() const
{
    return new WximageFile(static_cast<const WximageFile&>(*this));
}

WximageFile::~WximageFile()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts WximageFile::getLength() const
{
    return Convert::timeToPts(Constants::sHour * 8); // 8 Hours...
}
// todo implement player autoupdate if the file is modified on disk.
void WximageFile::moveTo(pts position)
{
    // NOT: VideoFile::moveTo(position) NOR File::moveTo(position); - will cause crash in avcodec
}

void WximageFile::clean()
{
    mOutputFrame.reset();
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr WximageFile::getNextVideo(const VideoCompositionParameters& parameters)
{
    readImage();

    if (mOutputFrame == nullptr || 
        parameters.getBoundingBox() != mOutputFrame->getParameters().getBoundingBox())
    {
        wxImagePtr outputImage = boost::make_shared<wxImage>(mInputImage->Copy());
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

wxSize WximageFile::getSize()
{
    readImage();
    return mInputImage->GetSize();
}


//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void WximageFile::readImage()
{
    if (mInputImage == nullptr)
    {
        mInputImage = boost::make_shared<wxImage>(getPath().GetFullPath());
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const WximageFile& obj)
{
    os << static_cast<const VideoFile&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WximageFile::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoFile);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WximageFile::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WximageFile::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::WximageFile)