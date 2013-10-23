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

#include "Properties.h"

#include "Config.h"
#include "Project.h"
#include "Render.h"
#include "UtilClone.h"
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
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mFrameRate);
        ar & BOOST_SERIALIZATION_NVP(mVideoWidth);
        ar & BOOST_SERIALIZATION_NVP(mVideoHeight);
        ar & BOOST_SERIALIZATION_NVP(mAudioChannels);
        ar & BOOST_SERIALIZATION_NVP(mAudioFrameRate);
        ar & BOOST_SERIALIZATION_NVP(mDefaultRender);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void Properties::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Properties::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Properties)