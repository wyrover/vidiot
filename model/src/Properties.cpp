#include "Properties.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "UtilSerializeBoost.h"
#include "Config.h"
#include "Project.h"

namespace model {

Properties::Properties()
:   mFrameRate(framerate::fromString(Config::ReadString(Config::sPathDefaultFrameRate)))
,   mVideoWidth(Config::ReadLong(Config::sPathDefaultVideoWidth))
,   mVideoHeight(Config::ReadLong(Config::sPathDefaultVideoHeight))
{
}

Properties::~Properties()
{
}

// static
PropertiesPtr Properties::get()
{
    return Project::get().getProperties();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

FrameRate Properties::getFrameRate() const
{
    return mFrameRate;
}

wxSize Properties::getVideoSize() const
{
    return wxSize(mVideoWidth,mVideoHeight);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Properties::serialize(Archive & ar, const unsigned int version)
{
    ar & mFrameRate;
    ar & mVideoWidth;
    ar & mVideoHeight;
}

template void Properties::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Properties::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace