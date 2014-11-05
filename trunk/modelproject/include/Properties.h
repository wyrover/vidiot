// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "UtilFrameRate.h"
#include "UtilSingleInstance.h"

namespace model {

class Properties
    :   public SingleInstance<Properties>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Properties();

    /// Constructor used for tests.
    /// This constructor allows specifying the values used.
    Properties(const FrameRate& fr);

    Properties(const Properties& other);

    virtual ~Properties();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    FrameRate getFrameRate() const;
    void setFrameRate(FrameRate frameRate);

    wxSize getVideoSize() const;
    void setVideoSize(wxSize size);

    /// \return number of audio channels, thus the number of independent speakers
    int getAudioNumberOfChannels() const; 
    
    void setAudioNumberOfChannels(int channels);

    /// \return sample rate used for audio
    int getAudioSampleRate() const; 
    
    void setAudioSampleRate(int audioFrameRate);

    /// \return clone of the currently set default render
    render::RenderPtr getDefaultRender() const;

    /// Sets the default render to be a clone of the given render
    void setDefaultRender(const render::RenderPtr& render);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FrameRate mFrameRate;
    long mVideoWidth;
    long mVideoHeight;
    int mAudioChannels;
    int mAudioSampleRate;
    render::RenderPtr mDefaultRender;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Properties, 3)
BOOST_CLASS_EXPORT_KEY(model::Properties)

#endif
