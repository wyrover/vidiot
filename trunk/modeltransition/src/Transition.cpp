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

#include "Transition.h"

#include "ClipEvent.h"
#include "Enums.h"
#include "Track.h"
#include "TransitionParameter.h"
#include "TransitionParameterBool.h"
#include "TransitionParameterColor.h"
#include "TransitionParameterDirection.h"
#include "TransitionParameterDouble.h"
#include "TransitionParameterImage.h"
#include "TransitionParameterInt.h"
#include "TransitionParameterRotationDirection.h"
#include "UtilSerializeBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Transition::Transition()
    : Clip()
    , mFramesLeft(boost::none)
    , mFramesRight(boost::none)
    , mParameters()
{
    VAR_DEBUG(*this);
}

void Transition::init(boost::optional<pts> nFramesLeft, boost::optional<pts> nFramesRight)
{
    mFramesLeft = nFramesLeft;
    mFramesRight = nFramesRight;
    ASSERT_MORE_THAN_ZERO(getLength());

    initParameters(getCurrentParameters());
    for (auto parameter : mParameters)
    {
        parameter.second->setOnChanged(std::bind(&Transition::onParameterChanged, this, std::placeholders::_1));
    }

    VAR_DEBUG(this)(nFramesLeft)(nFramesRight);
}

Transition::Transition(const Transition& other)
    : Clip(other)
    , mFramesLeft(other.mFramesLeft)
    , mFramesRight(other.mFramesRight)
    , mParameters(make_cloned<wxString, TransitionParameter>(other.mParameters))
{
    VAR_DEBUG(other)(*this);
    for (auto parameter : mParameters)
    {
        parameter.second->setOnChanged(std::bind(&Transition::onParameterChanged, this, std::placeholders::_1));
    }
}

Transition::~Transition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

wxString Transition::getDescription() const
{
    if (!mFramesLeft)
    {
        return getDescription(TransitionTypeFadeIn);
    }
    if (!mFramesRight)
    {
        return getDescription(TransitionTypeFadeOut);
    }
    return getDescription(TransitionTypeFadeOutToNext);
}

pts Transition::getLength() const
{
    ASSERT(mFramesLeft || mFramesRight);
    return (mFramesLeft ? *mFramesLeft : 0) + (mFramesRight ? *mFramesRight : 0);
}

void Transition::moveTo(pts position)
{
    setNewStartPosition(position);
}

void Transition::clean()
{
    Clip::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void Transition::setLink(const IClipPtr& link)
{
     ASSERT(!link)(link); // Transitions may never be linked to anything
}

pts Transition::getMinAdjustBegin() const
{
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    pts result = std::numeric_limits<pts>().min();
    if (getLeft())
    {
        ASSERT(getPrev()); // Avoid bugs where this method is called before a transition has been made part of a track
        result = -1 *  getPrev()->getLength();
        if (getRight())
        {
            ASSERT(getNext());
            result = std::max(result, getNext()->getMinAdjustBegin());
        }
    }
    else
    {
        result = 0; // InOnlyTransition: Cannot enlarge to the left
    }
    return result;
}

pts Transition::getMaxAdjustBegin() const
{
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    return (mFramesLeft ? *mFramesLeft : 0);
}

void Transition::adjustBegin(pts adjustment)
{
    VAR_DEBUG(adjustment)(*this);
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    ASSERT(mFramesLeft);
    mFramesLeft.reset(*mFramesLeft - adjustment);
}

pts Transition::getMinAdjustEnd() const
{
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    return (mFramesRight ? -1 * *mFramesRight : 0);
}

pts Transition::getMaxAdjustEnd() const
{
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    pts result = std::numeric_limits<pts>().max();
    if (getRight())
    {
        ASSERT(getNext()); // Avoid bugs where this method is called before a transition has been made part of a track
        result = getNext()->getLength();
        if (getLeft())
        {
            ASSERT(getPrev());
            result = std::min(result, getPrev()->getMaxAdjustEnd());
        }
    }
    else // OutOnlyTransition: Cannot enlarge to the right
    {
        result = 0;
    }
    return result;
}

void Transition::adjustEnd(pts adjustment)
{
    VAR_DEBUG(adjustment)(*this);
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    ASSERT(mFramesRight);
    mFramesRight.reset(*mFramesRight + adjustment);
}

std::set<pts> Transition::getCuts(const std::set<IClipPtr>& exclude) const
{
    std::set<pts> result;
    result.insert(getLeftPts());
    result.insert(getTouchPosition());
    result.insert(getRightPts());
    return result;
}

std::ostream& Transition::dump(std::ostream& os) const
{
    os << *this;
    return os;
}

const char* Transition::getType() const
{
    return "Trans";
}

FilePtr Transition::getFile() const
{
    return FilePtr();
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

pts Transition::getTouchPosition() const
{
    return getLeftPts() + (mFramesLeft ? *mFramesLeft : 0);
}

boost::optional<pts> Transition::getLeft() const
{
    return mFramesLeft;
}

boost::optional<pts> Transition::getRight() const
{
    return mFramesRight;
}

TransitionType Transition::getTransitionType() const
{
    return
        mFramesLeft && mFramesRight ? TransitionTypeFadeOutToNext :
        mFramesLeft ? TransitionTypeFadeOut :
        TransitionTypeFadeIn;
}

model::IClipPtr Transition::makeLeftClip()
{
    model::IClipPtr result;
    if (getLeft())
    {
        ASSERT(getPrev());
        result = make_cloned<model::IClip>(getPrev());
        result->adjustBegin(result->getLength());
        result->adjustEnd(getLength());
    }
    return result;
}

model::IClipPtr Transition::makeRightClip()
{
    model::IClipPtr result;
    if (getRight())
    {
        ASSERT(getNext());
        result = make_cloned<model::IClip>(getNext());
        result->adjustEnd(- result->getLength());
        result->adjustBegin(-getLength());
    }
    return result;
}

bool Transition::supports(TransitionType type) const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

void Transition::initParameters(std::map<wxString, TransitionParameterPtr> currentValues)
{
    mParameters.clear(); // Start with nothing
    for (auto tuple : getAvailableParameters())
    {
        wxString name{ std::get<0>(tuple) };
        wxString description{ std::get<1>(tuple) };
        model::TransitionParameterPtr parameter{ std::get<2>(tuple) };

        parameter->setName(name);
        parameter->setDescription(description);

        if (currentValues.find(name) != currentValues.end())
        {
            // Use the value from the save file, if present
            parameter->copyValue(currentValues.find(name)->second);
        }
        else
        {
            // Use the default value
        }

        mParameters[name] = parameter;
        // todo ensure proper ordering of parameter widgets by adding an index to getParameters (or by having it return a vector?) and storing the index just like description, and then retrieve the ordered list in DetailsClip::createTransitionParameterWidgets
    }
}

std::map<wxString, TransitionParameterPtr> Transition::getCurrentParameters() const
{
    return mParameters;
}

std::vector<TransitionParameterPtr> Transition::getSortedParameters() const
{
    std::vector<TransitionParameterPtr> result;
    for (auto tuple : getAvailableParameters())
    {
        wxString name{ std::get<0>(tuple) };
        ASSERT_MAP_CONTAINS(mParameters, name);
        auto it{ mParameters.find(name) };
        result.push_back(it->second);
    }
    return result;
}
    
template <typename PARAMETERTYPE>
boost::shared_ptr<PARAMETERTYPE> Transition::getParameter(wxString name) const
{
    ASSERT_MAP_CONTAINS(mParameters, name);
    boost::shared_ptr<TransitionParameter> parameter{ mParameters.find(name)->second };
    boost::shared_ptr<PARAMETERTYPE> result{ boost::dynamic_pointer_cast<PARAMETERTYPE>(parameter) };
    ASSERT_NONZERO(result)(parameter);
    return result;
}

template boost::shared_ptr<TransitionParameterBool> Transition::getParameter<TransitionParameterBool>(wxString name) const;
template boost::shared_ptr<TransitionParameterDouble> Transition::getParameter<TransitionParameterDouble>(wxString name) const;
template boost::shared_ptr<TransitionParameterImage> Transition::getParameter<TransitionParameterImage>(wxString name) const;
template boost::shared_ptr<TransitionParameterInt> Transition::getParameter<TransitionParameterInt>(wxString name) const;
template boost::shared_ptr<TransitionParameterColor> Transition::getParameter<TransitionParameterColor>(wxString name) const;
template boost::shared_ptr<TransitionParameterDirection> Transition::getParameter<TransitionParameterDirection>(wxString name) const;
template boost::shared_ptr<TransitionParameterRotationDirection> Transition::getParameter<TransitionParameterRotationDirection>(wxString name) const;

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Transition& obj)
{
    // Keep order same as Clip for 'dump' method
    os << static_cast<const Clip&>(obj) << '|'
       << std::setw(6) << obj.mFramesLeft << '|'
       << std::setw(6) << obj.mFramesRight << '|'
       << obj.mParameters;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Transition::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Clip);
        if (version == 1)
        {
            pts left = 0;
            pts right = 0;
            ar & boost::serialization::make_nvp("mFramesLeft",left);
            ar & boost::serialization::make_nvp("mFramesRight",right);
            if (left == 0) // Out-only transition
            {
                mFramesLeft.reset();
            }
            else
            {
                mFramesLeft.reset(left);
            }
            if (right == 0) // In-only
            {
                mFramesRight.reset(); // In-only transition
            }
            else
            {
                mFramesRight.reset(right);
            }
        }
        else
        {
            ar & BOOST_SERIALIZATION_NVP(mFramesLeft);
            ar & BOOST_SERIALIZATION_NVP(mFramesRight);
        }
        if (version == 3)
        {
            std::map<int, TransitionParameterPtr> parameters;
            ar & boost::serialization::make_nvp("mParameters", parameters);
            for (auto kvp : parameters)
            {
                mParameters.insert(std::make_pair("color", kvp.second));
            }
        }
        if (version > 3)
        {
            ar & BOOST_SERIALIZATION_NVP(mParameters);
        }
        ASSERT_MORE_THAN_ZERO(getLength());
        // NOT: mSelected. After loading, nothing is selected.
        if (Archive::is_loading::value)
        {
            initParameters(mParameters);
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Transition::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Transition::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Transition)
