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

#ifndef MODEL_TRANSITION_H
#define MODEL_TRANSITION_H

#include "Clip.h"

namespace model {

/// Class representing transitions in the timeline. Note that the actual transition
/// is rendered by taking its adjacent clips, cloning these, and adjusting the clones
/// to provide the correct frames.
///
/// When transitions are added to a track, the adjacent clips are shortened in such a
/// way that they 'make room' for the transition. A clips offset is increased and/or
/// its length is reduced.
class Transition
    :   public Clip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk and for creating a new transition
    /// Serialize will be called after construction for recovery from disk.
    /// Init must be called for new transitions.
    Transition();

    void init(pts nFramesLeft, pts nFramesRight);

    ///< Used for making deep copies (clones)
    virtual Transition* clone() const override;

    virtual ~Transition();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    pts getLength() const override;
    void moveTo(pts position) override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    void setLink(IClipPtr link) override;

    pts getMinAdjustBegin() const override;
    pts getMaxAdjustBegin() const override;
    void adjustBegin(pts adjustment) override;

    pts getMinAdjustEnd() const override;
    pts getMaxAdjustEnd() const override;
    void adjustEnd(pts adjustment) override;

    std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

    virtual std::ostream& dump(std::ostream& os) const override;
    virtual char* getType() const override;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    pts getTouchPosition() const;   ///< \return position where the two transitioned clips are 'touching'
    pts getLeft() const;    ///< \return number of frames to the left of the cut between the two clips
    pts getRight() const;   ///< \return number of frames to the right of the cut between the two clips

    /// Make the 'in' clip that is to be used for rendering data
    /// This takes the previous clip in the track, clones it, and adjust the
    /// clone such that it has the offset and length as required for the
    /// transition.
    /// If there is no 'in' clip for this transition (out-only transition) then
    /// returns '0'.
    /// \return a clone of the clip to be used for rendering transition data
    model::IClipPtr makeLeftClip() const;

    /// Make the 'out' clip that is to be used for rendering data
    /// This takes the previous clip in the track, clones it, and adjust the
    /// clone such that it has the offset and length as required for the
    /// transition.
    /// If there is no 'out' clip for this transition (in-only transition) then
    /// returns '0'.
    /// \return a clone of the clip to be used for rendering transition data
    model::IClipPtr makeRightClip() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    Transition(const Transition& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mFramesLeft;    ///< Number of frames to the left of the cut between the two clips
    pts mFramesRight;   ///< Number of frames to the right of the cut between the two clips

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Transition& obj );

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
BOOST_CLASS_VERSION(model::Transition, 1)

#endif // MODEL_TRANSITION_H