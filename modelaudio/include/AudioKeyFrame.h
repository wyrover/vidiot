// Copyright 2015-2016 Eric Raijmakers.
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

#include "KeyFrame.h"
#include "Enums.h"

namespace model {

typedef std::map<pts, AudioKeyFramePtr> AudioKeyFrameMap;

class AudioKeyFrame
    : public KeyFrame
{
public:

    static constexpr int sVolumeMin = 0;
    static constexpr int sVolumeMax = 200;
    static constexpr int sVolumeDefault = 100;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioKeyFrame();

    AudioKeyFrame(AudioKeyFramePtr before, AudioKeyFramePtr after, pts positionBefore, pts position, pts positionAfter);

    virtual AudioKeyFrame* clone() const override;

    virtual ~AudioKeyFrame();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void setVolume(int volume);
    int getVolume() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    AudioKeyFrame(const AudioKeyFrame& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mVolume;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const AudioKeyFrame& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::AudioKeyFrame, 1)
BOOST_CLASS_EXPORT_KEY(model::AudioKeyFrame)
