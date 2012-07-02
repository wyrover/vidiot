#include "Properties.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "Config.h"
#include "Project.h"
#include "Render.h"
#include "UtilCloneable.h"
#include "UtilSerializeBoost.h"

namespace model {

const int sStereo = 2;
const int sAudioFrameRate = 44100;

Properties::Properties()
:   mFrameRate(framerate::fromString(Config::ReadString(Config::sPathDefaultFrameRate)))
,   mVideoWidth(Config::ReadLong(Config::sPathDefaultVideoWidth))
,   mVideoHeight(Config::ReadLong(Config::sPathDefaultVideoHeight))
,   mAudioChannels(sStereo)
,   mAudioFrameRate(sAudioFrameRate)
,   mDefaultRender(boost::make_shared<model::render::Render>())
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

int Properties::getAudioNumberOfChannels() const
{
    return mAudioChannels;
}

int Properties::getAudioFrameRate() const
{
    return mAudioFrameRate;
}

render::RenderPtr Properties::getDefaultRender() const
{
    return make_cloned<render::Render>(mDefaultRender);
}

void Properties::setDefaultRender(render::RenderPtr render)
{
    mDefaultRender = make_cloned<render::Render>(render);
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
    ar & mAudioChannels;
    ar & mAudioFrameRate;
    ar & mDefaultRender;
}

template void Properties::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Properties::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace