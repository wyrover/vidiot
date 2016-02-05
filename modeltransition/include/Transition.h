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

#include "Clip.h"

namespace model {

enum TransitionType : int;

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

    void init(boost::optional<pts> nFramesLeft, boost::optional<pts> nFramesRight);

    ///< Used for making deep copies (clones)
    virtual Transition* clone() const override = 0;

    virtual ~Transition();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    wxString getDescription() const override;
    pts getLength() const override;
    void moveTo(pts position) override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    void setLink(const IClipPtr& link) override;

    pts getMinAdjustBegin() const override;
    pts getMaxAdjustBegin() const override;
    void adjustBegin(pts adjustment) override;

    pts getMinAdjustEnd() const override;
    pts getMaxAdjustEnd() const override;
    void adjustEnd(pts adjustment) override;

    std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

    virtual std::ostream& dump(std::ostream& os) const override;
    virtual const char* getType() const override;

    FilePtr getFile() const override;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    /// \return position where the two transitioned clips are 'touching'
    pts getTouchPosition() const;

    /// \see mFramesLeft
    boost::optional<pts> getLeft() const;

    /// \see mFramesRight
    boost::optional<pts> getRight() const;

    TransitionType getTransitionType() const;

    std::vector<wxWindow*> makeParameterWidgets(wxWindow* parent);
    void destroyParameterWidgets();

    /// Make the 'in' clip that is to be used for rendering data
    /// This takes the previous clip in the track, clones it, and adjust the
    /// clone such that it has the offset and length as required for the
    /// transition.
    /// If there is no 'in' clip for this transition (out-only transition) then
    /// returns '0'.
    /// \return a clone of the clip to be used for rendering transition data
    virtual model::IClipPtr makeLeftClip();

    /// Make the 'out' clip that is to be used for rendering data
    /// This takes the previous clip in the track, clones it, and adjust the
    /// clone such that it has the offset and length as required for the
    /// transition.
    /// If there is no 'out' clip for this transition (in-only transition) then
    /// returns '0'.
    /// \return a clone of the clip to be used for rendering transition data
    virtual model::IClipPtr makeRightClip();

    /// \return the name to be used for the transition, given the transition type.
    virtual wxString getDescription(TransitionType type) const = 0;

    /// \return true if the given type of transition is supported by this transition
    /// Default implementation supports all types
    virtual bool supports(TransitionType type) const;

    //////////////////////////////////////////////////////////////////////////
    // PARAMETERS
    //////////////////////////////////////////////////////////////////////////

    /// Initalize all parameters with proper values.
    /// Default param is a copy of the current parameter list.
    /// This is used for initialization after reading from disk.
    void initParameters(std::map<wxString, TransitionParameterPtr> currentValues);

    /// \note Ordering as returned here is the ordering in which the widgets must be shown.
    /// \return Possible parameters (name, description, parameter) holding default values. 
    virtual std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> getAvailableParameters() const = 0;

    /// \return Currently configured parameters for this transition
    std::map<wxString, TransitionParameterPtr> getCurrentParameters() const;

    /// \return Sorted list of currently configured parameters, for creating widgets in the proper order.
    std::vector<TransitionParameterPtr> getSortedParameters() const;

    template <typename PARAMETERTYPE>
    boost::shared_ptr<PARAMETERTYPE> getParameter(wxString name) const;

    virtual void onParameterChanged(const wxString& name) {};

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

    /// Out-only transition:
    ///     Unset optional in case this is an out-only transition.
    ///
    /// Crossfading transition:
    ///     Number of frames visible (from user point of view)
    ///     to the left of the cut between the two clips.
    ///
    /// Can be a 'set' optional with value '0':
    ///     Left clip IS part of the transition but from a user point
    ///     of view the transition only overlaps with the clip to the right.
    boost::optional<pts> mFramesLeft;

    /// In-only transition:
    ///     Unset optional in case this is an out-only transition.
    ///
    /// Crossfading transition:
    ///     Number of frames visible (from user point of view)
    ///     to the right of the cut between the two clips.
    ///
    /// Can be a 'set' optional with value '0':
    ///     Right clip IS part of the transition but from a user point
    ///     of view the transition only overlaps with the clip to the left.
    boost::optional<pts> mFramesRight;

    /// All parameters relevant for this transition.
    std::map<wxString, TransitionParameterPtr> mParameters;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Transition& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::Transition, 4)
BOOST_CLASS_EXPORT_KEY(model::Transition)
