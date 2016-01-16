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

#pragma once

#include "AudioTransition.h"

namespace model { namespace audio { namespace transition {

class CrossFade
    :   public AudioTransition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CrossFade() = default;

    virtual CrossFade* clone() const;

    virtual ~CrossFade() = default;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> getParameters() const override { return{}; };

    wxString getDescription(TransitionType type) const override;

    //////////////////////////////////////////////////////////////////////////
    // AUDIOTRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getAudio(pts position, const IClipPtr& leftClip, const IClipPtr& rightClip, const AudioCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    CrossFade(const CrossFade& other) = default;

private:

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}}} // namespace

BOOST_CLASS_VERSION(model::audio::transition::CrossFade, 1)
BOOST_CLASS_EXPORT_KEY(model::audio::transition::CrossFade)
