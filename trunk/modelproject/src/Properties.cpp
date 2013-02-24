#include "Properties.h"

#include "Config.h"
#include "Project.h"
#include "Render.h"
#include "UtilCloneable.h"
#include "UtilSerializeBoost.h"

namespace model {

Properties::Properties()
:   mFrameRate(Config::ReadString(Config::sPathDefaultFrameRate))
,   mVideoWidth(Config::ReadLong(Config::sPathDefaultVideoWidth))
,   mVideoHeight(Config::ReadLong(Config::sPathDefaultVideoHeight))
,   mAudioChannels(Config::ReadLong(Config::sPathDefaultAudioChannels))
,   mAudioFrameRate(Config::ReadLong(Config::sPathDefaultAudioSampleRate))
,   mDefaultRender(boost::make_shared<model::render::Render>())
{
    VAR_DEBUG(this);
}

Properties::Properties(FrameRate fr)
:   mFrameRate(fr)
,   mVideoWidth(100)
,   mVideoHeight(100)
,   mAudioChannels(1)
,   mAudioFrameRate(44100)
,   mDefaultRender()
{
    VAR_DEBUG(this);
}

Properties::~Properties()
{
    VAR_DEBUG(this);
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