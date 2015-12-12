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
#include "UtilRational.h"

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

    virtual void setSpeed(const rational64& speed);
    virtual rational64 getSpeed() const;

    //////////////////////////////////////////////////////////////////////////
    // KEY FRAMES
    //////////////////////////////////////////////////////////////////////////

    /// \return list of actual possible key frames. These are the key frames that are in the region 'visible' in the timeline.
    /// \note Returned pts'es are related to the input, thus no offset is applied! 
    ///       However, the speed has been applied! 
    ///       Rationale: that allows proper 'target' ptses, but also allows having keyframes under transitions.
    /// \note returned pts'es include positions 'under' any adjacent transitions
    /// \note If there are no key frames only contains the [-1] default key frame.
    std::map<pts, KeyFramePtr> getKeyFrames() const;

    /// \return the always present default key frame (the parameters used when there are no key frames)
    KeyFramePtr getDefaultKeyFrame() const;

    /// \return (clone of) key frame (possibly interpolated) at given (output) position
    /// \param offset offset with respect to the output
    KeyFramePtr getFrameAt(pts offset) const;

    /// Add a key frame
    /// \param offset offset into the input data
    /// \param frame new key frame
    void addKeyFrameAt(pts offset, KeyFramePtr frame);

    /// Remove a key frame
    /// \param offset offset into the input data
    void removeKeyFrameAt(pts offset);

    //////////////////////////////////////////////////////////////////////////
    // FOR PREVIEWING
    //////////////////////////////////////////////////////////////////////////

    pts getOffset() const;
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

    //////////////////////////////////////////////////////////////////////////
    // KEY FRAMES
    //////////////////////////////////////////////////////////////////////////

    void setDefaultKeyFrame(KeyFramePtr keyframe);

    virtual KeyFramePtr interpolate(KeyFramePtr before, KeyFramePtr after, pts positionBefore, pts position, pts positionAfter) const = 0;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IFilePtr mRender;   ///< The producer of audiovisual data for this clip

    rational64 mSpeed; ///< Speed for rendering. A speed != 1 implies that the speed of of frames of mRender is changed with the given speed. Offset and length are applied AFTER applying the speed.
    pts mOffset;        ///< Offset in 'sequence' speed and time base; number of frames to skip from the original media file (after applying speed - to skip).
    pts mLength;        ///< Length of the clip in 'sequence' speed and time base; number of frames to show from the original media file (after applying speed).

    mutable wxString mDescription;  ///< Stored for performance (cached) and for easier debugging.

    /// Keyframes are stored with a position relative to the input.
    /// Thus, trimming has no effect on the (member) list of key frames.
    std::map<pts, KeyFramePtr> mKeyFrames;

    /// Parameters used when no keyframes are present
    KeyFramePtr mDefaultKeyFrame;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    static wxString stripDescription(const wxString& description);

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

BOOST_CLASS_VERSION(model::ClipInterval, 4)
BOOST_CLASS_EXPORT_KEY(model::ClipInterval)
