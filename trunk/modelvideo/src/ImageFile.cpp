#include "ImageFile.h"

#include "UtilLog.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ImageFile::ImageFile()
:	VideoFile()
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(wxFileName path)
:	VideoFile(path)
{
    VAR_DEBUG(*this);
}

ImageFile::ImageFile(const ImageFile& other)
:   VideoFile(other)
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

void ImageFile::moveTo(pts position)
{
    // NOT: VideoFile::moveTo(position); - will cause crash in avcodec
}

void ImageFile::clean()
{
    VAR_DEBUG(this);
    VideoFile::clean();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr ImageFile::getNextVideo(const VideoCompositionParameters& parameters)
{
    // todo
    return VideoFile::getNextVideo(parameters); // todo idee: bij aanmaken het ene frame meteen ophalen met de originele grootte en dan cachen?
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

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