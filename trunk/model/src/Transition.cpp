#include "Transition.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "Transition.h"

namespace model {

Transition::Transition()
    :   IControl()
    ,   mFramesLeft(0)
    ,   mFramesRight(0)
    ,   mLastSetPosition(boost::none)
{
    VAR_DEBUG(this);
}

Transition::Transition(pts nFramesLeft, pts nFramesRight)
    :   IControl()
    ,   mFramesLeft(nFramesLeft)
    ,   mFramesRight(nFramesRight)
    ,   mLastSetPosition(boost::none)
{
    VAR_DEBUG(this);
}

Transition::Transition(const Transition& other)
    :	IControl()
    ,   mFramesLeft(other.mFramesLeft)
    ,   mFramesRight(other.mFramesRight)
    ,   mLastSetPosition(boost::none)
{
    VAR_DEBUG(*this);
}

Transition* Transition::clone()
{ 
    return new Transition(static_cast<const Transition&>(*this)); 
}

Transition::~Transition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Transition::getLength()
{
    return mFramesLeft + mFramesRight;
}

void Transition::moveTo(pts position)
{
    mLastSetPosition.reset(position);
    // todo
}

wxString Transition::getDescription() const
{
    return "";
}

void Transition::clean()
{

}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

pts Transition::getLeft() const
{
    return mFramesLeft;
}

pts Transition::getRight() const
{
    return mFramesRight;
}

//////////////////////////////////////////////////////////////////////////
// CURRENT POSITION HANDLING
//////////////////////////////////////////////////////////////////////////

void Transition::invalidateLastSetPosition()
{
    mLastSetPosition = boost::none;
}

boost::optional<pts> Transition::getLastSetPosition() const
{
    return mLastSetPosition;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Transition& obj )
{
    os << &obj << '|' << obj.mFramesLeft << '|' << obj.mFramesRight;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Transition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
}
template void Transition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Transition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
