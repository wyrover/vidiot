// Copyright 2013-2016 Eric Raijmakers.
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
#include "UtilSerializeBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Properties::Properties()
:   mFrameRate(Config::ReadString(Config::sPathVideoDefaultFrameRate))
,   mVideoWidth(Config::ReadLong(Config::sPathVideoDefaultWidth))
,   mVideoHeight(Config::ReadLong(Config::sPathVideoDefaultHeight))
,   mAudioChannels(Config::ReadLong(Config::sPathAudioDefaultNumberOfChannels))
,   mAudioSampleRate(Config::ReadLong(Config::sPathAudioDefaultSampleRate))
,   mDefaultRender(boost::make_shared<model::render::Render>())
{
    VAR_DEBUG(this);
}

Properties::Properties(const FrameRate& fr)
:   mFrameRate(fr)
,   mVideoWidth(100)
,   mVideoHeight(100)
,   mAudioChannels(1)
,   mAudioSampleRate(44100)
,   mDefaultRender()
{
    VAR_DEBUG(this);
}

// todo threading problems. Properties must only be accessed in main thread.
// - all getters check for this alike Config.
// - then model::Convert must be adjusted to never use the framerate et.al. from Properties singleton
// - then if properties changed during rendering/playback, the original values are still used.
// - Maybe all access to 'singleinstance' only allowed in main thread. Then, pass (for example)
//   composition parameters around in secondary threads, which contain values copied
//   during start of playback/rendering.
// - test edit props during playback (should stop all playback then?)
// - When properties are updated (audio sample rate), invalidate all entries for peaks in metadatacache,
//   and redraw all peaks.

Properties::Properties(const Properties& other)
    : mFrameRate(other.mFrameRate)
    , mVideoWidth(other.mVideoWidth)
    , mVideoHeight(other.mVideoHeight)
    , mAudioChannels(other.mAudioChannels)
    , mAudioSampleRate(other.mAudioSampleRate)
    , mDefaultRender()
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

void Properties::setFrameRate(FrameRate frameRate)
{
    mFrameRate = frameRate;
}

wxSize Properties::getVideoSize() const
{
    return wxSize(mVideoWidth,mVideoHeight);
}

void Properties::setVideoSize(wxSize size)
{
    mVideoWidth = size.GetWidth();
    mVideoHeight = size.GetHeight();
}

int Properties::getAudioNumberOfChannels() const
{
    return mAudioChannels;
}

void Properties::setAudioNumberOfChannels(int channels)
{
    mAudioChannels = channels;
}

int Properties::getAudioSampleRate() const
{
    return mAudioSampleRate;
}

void Properties::setAudioSampleRate(int audioFrameRate)
{
    mAudioSampleRate = audioFrameRate;
}

render::RenderPtr Properties::getDefaultRender() const
{
    return make_cloned<render::Render>(mDefaultRender);
}

void Properties::setDefaultRender(const render::RenderPtr& render)
{
    mDefaultRender = make_cloned<render::Render>(render);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

struct OldFrameRate
    : public boost::rational<int>
{
    OldFrameRate(int num, int den) : boost::rational<int>(num,den) {}
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        try
        {
            ar & boost::serialization::make_nvp( "rational", boost::serialization::base_object< boost::rational<int> >(*this));
        }
        catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
        catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
        catch (...)                                  { LOG_ERROR;                                   throw; }
    }

};

template<class Archive>
void Properties::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        if (version == 1 && Archive::is_loading::value)
        {
            OldFrameRate tmp(1,1);
            ar & boost::serialization::make_nvp( "mFrameRate", tmp );
            mFrameRate.assign(static_cast<int64_t>(tmp.numerator()), static_cast<int64_t>(tmp.denominator()));
        }
        else
        {
            ar & BOOST_SERIALIZATION_NVP(mFrameRate);
        }
        ar & BOOST_SERIALIZATION_NVP(mVideoWidth);
        ar & BOOST_SERIALIZATION_NVP(mVideoHeight);
        ar & BOOST_SERIALIZATION_NVP(mAudioChannels);
        if (version < 3 && Archive::is_loading::value)
        {
            int mAudioFrameRate = 0;
            ar & BOOST_SERIALIZATION_NVP(mAudioFrameRate);
            mAudioSampleRate = mAudioFrameRate;
        }
        else
        {
            ar & BOOST_SERIALIZATION_NVP(mAudioSampleRate);
        }
        ar & BOOST_SERIALIZATION_NVP(mDefaultRender);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void Properties::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Properties::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Properties)