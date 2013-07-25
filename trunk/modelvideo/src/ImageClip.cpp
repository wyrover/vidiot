#include "ImageClip.h"

#include "Config.h"
#include "ImageFile.h"
#include "UtilLog.h"
#include "VideoCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ImageClip::ImageClip()
    : VideoClip()
{
    VAR_DEBUG(*this);
}

ImageClip::ImageClip(VideoFilePtr file)
    : VideoClip(file)
{
    VAR_DEBUG(*this);
    ASSERT(file->isA<ImageFile>())(file);
    ASSERT_DIFFERS(file->getLength(),1);

    pts length = getLength();
    pts half = length / 2;
    pts remainingLength = Config::ReadLong(Config::sPathDefaultStillImageLength);

    // Move right edge to the left such that the clip can be extended if required
    adjustEnd( - half );

    // Move left edge to the right sich that the clip can be extended if required. The -remainingLength ensures that the resulting clip has the correct resulting size
    adjustBegin(half - remainingLength);

    ASSERT_EQUALS(getLength(),remainingLength);
}

ImageClip::~ImageClip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ImageClip& obj )
{
    os << static_cast<const VideoClip&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void ImageClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<VideoClip>(*this);
}
template void ImageClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void ImageClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace