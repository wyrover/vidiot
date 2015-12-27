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

typedef std::map<pts, KeyFramePtr> KeyFrameMap;

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
    /// \note Returned pts'es are relative to getPerceivedOffset(), taking the speed into account.
    ///       Rationale: that allows proper 'target' ptses, but also allows having keyframes under transitions.
    /// \note returned pts'es include positions 'under' any adjacent transitions
    /// \note May be empty if there are no specific key frames.
    KeyFrameMap getKeyFramesOfPerceivedClip() const;

    /// Return the minimum and maximum positions allowed for a key frame
    /// \param index number of the key frame
    /// \return (min,max) allowed positions (cannot move 'over' other key frames)
    std::pair<pts, pts> getKeyFrameBoundaries(size_t index) const;

    /// \return the position (replative to 'getPerceivedOffset()') of the given key frame
    /// \param index number of the key frame
    pts getKeyFramePosition(size_t index) const;

    /// Move a key frame backward or forward in time
    /// \param index number of the key frame to be moved
    /// \param offset new position (relative to 'getPerceivedOffset()')
    void setKeyFramePosition(size_t index, pts offset);

    /// \return the always present default key frame (the parameters used when there are no key frames)
    KeyFramePtr getDefaultKeyFrame() const;

    /// \return (clone of) key frame (possibly interpolated) at given (output) position
    /// \param offset offset with respect to 'getPerceivedOffset()'
    KeyFramePtr getFrameAt(pts offset) const;

    /// Add a key frame
    /// \param offset offset with respect to  'getPerceivedOffset()'
    /// \param frame new key frame
    void addKeyFrameAt(pts offset, KeyFramePtr frame);

    /// Remove a key frame
    /// \param offset offset with respect to 'getPerceivedOffset()'
    void removeKeyFrameAt(pts offset);

    //////////////////////////////////////////////////////////////////////////
    // OFFSET
    //////////////////////////////////////////////////////////////////////////

    pts getOffset() const;

    /// Return the offset of the clip as viewed by the user.
    /// The difference between the actual offset of the clip (getOffset()) and
    /// this perceived offset is the area of the clip that is part of one of
    /// its adjacent transitions; basically, the part of the clip that is shown
    /// 'under' a transition in the timeline.
    pts getPerceivedOffset() const;

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

    /// Remove any key frames that are no longer in the 'perceived clip area'.
    void pruneKeyFrames();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IFilePtr mRender;   ///< The producer of audiovisual data for this clip

    rational64 mSpeed; ///< Speed for rendering. A speed != 1 implies that the speed of of frames of mRender is changed with the given speed. Offset and length are applied AFTER applying the speed.
    pts mOffset;        ///< Offset in 'sequence' speed and time base; number of frames to skip from the original media file (after applying speed - to skip).
    pts mLength;        ///< Length of the clip in 'sequence' speed and time base; number of frames to show from the original media file (after applying speed).

    mutable wxString mDescription;  ///< Stored for performance (cached) and for easier debugging.

    /// Keyframes are stored with a position relative to the input, using the input speed.
    /// Thus, trimming has no effect on the (member) list of key frames.
    /// Rationale: This allows changing speed, and trimming without having to adjust this data structure for the trimming.
    /// Note that the data structure is adjusted during trimming, but only for removing 'invisible' key frames. 
    /// The adjustments are not required to maintain a correct key frame structure. 
    KeyFrameMap mKeyFrames;

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
