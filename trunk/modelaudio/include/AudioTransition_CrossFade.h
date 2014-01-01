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

#ifndef AUDIO_TRANSITION_CROSSFADE_H
#define AUDIO_TRANSITION_CROSSFADE_H

#include "AudioTransition.h"

namespace model { namespace audio { namespace transition {

struct Cache;

class CrossFade
    :   public AudioTransition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CrossFade();

    virtual CrossFade* clone() const;

    virtual ~CrossFade();

    //////////////////////////////////////////////////////////////////////////
    // AUDIOTRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual void reset();
    virtual AudioChunkPtr getAudio(samplecount position, IClipPtr leftClip, IClipPtr rightClip, const AudioCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    CrossFade(const CrossFade& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::shared_ptr<Cache> mCache;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const CrossFade& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::audio::transition::CrossFade, 1)
BOOST_CLASS_EXPORT_KEY(model::audio::transition::CrossFade)

#endif // AUDIO_TRANSITION_CROSSFADE_H