// Copyright 2013-2015 Eric Raijmakers.
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

#include "Clip.h"

namespace model {

/// Class responsible for handling clips that are (parts of) multimedia clips that have a fixed length.
/// Those clips are audio/video files (but not images), and sequences.
class ClipInterval
    :   public Clip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ClipInterval();                   ///< Constructor for recovery from disk
    ClipInterval(const IFilePtr& render);    ///< Constructor for creating new clip from other asset
    virtual ~ClipInterval();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual pts getMinAdjustBegin() const override;
    virtual pts getMaxAdjustBegin() const override;
    virtual void adjustBegin(pts adjustment) override;

    virtual pts getMinAdjustEnd() const override;
    virtual pts getMaxAdjustEnd() const override;
    virtual void adjustEnd(pts adjustment) override;

    //////////////////////////////////////////////////////////////////////////
    // SPEED
    //////////////////////////////////////////////////////////////////////////

    virtual void setSpeed(const boost::rational<int>& speed);
    virtual boost::rational<int> getSpeed() const;

    //////////////////////////////////////////////////////////////////////////
    // FOR PREVIEWING
    //////////////////////////////////////////////////////////////////////////

    pts getOffset();
    void maximize();

    //////////////////////////////////////////////////////////////////////////
    // ACCESS DATA GENERATOR
    //////////////////////////////////////////////////////////////////////////

    FilePtr getFile() const override;

    /// \return length of original input adjusted for speed
    pts getRenderLength() const;

    /// \return length of original input with 1/1 speed
    pts getRenderSourceLength() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // ACCESS RENDERING OBJECT
    //////////////////////////////////////////////////////////////////////////

    /// Method needed to convert the IControl instance mRender to an IVideo
    /// or an IAudio instance.
    template <class GENERATOR>
    boost::shared_ptr<GENERATOR> getDataGenerator()
    {
        return boost::static_pointer_cast<GENERATOR>(mRender);
    }

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    ClipInterval(const ClipInterval& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IFilePtr mRender;               ///< The producer of audiovisual data for this clip

    boost::rational<int> mSpeed;    ///< Speed for rendering. A speed != 1 implies that the speed of of frames of mRender is changed with the given speed. Offset and length are applied AFTER applying the speed.
    pts mOffset;                    ///< Offset in 'sequence' speed and time base; number of frames to skip from the original media file (after applying speed - to skip).
    pts mLength;                    ///< Length of the clip in 'sequence' speed and time base; number of frames to show from the original media file (after applying speed).

    mutable wxString mDescription;  ///< Stored for performance (cached) and for easier debugging.

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const ClipInterval& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::ClipInterval, 2)
BOOST_CLASS_EXPORT_KEY(model::ClipInterval)
