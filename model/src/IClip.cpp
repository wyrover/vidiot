#include "IClip.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilSerializeBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// TRACK
//////////////////////////////////////////////////////////////////////////

void IClip::setNext(IClipPtr next)
{
    mNext = next;
}

void IClip::setPrev(IClipPtr prev)
{
    mPrev = prev;
}

IClipPtr IClip::getNext()
{
    return mNext.lock();
}

IClipPtr IClip::getPrev()
{
    return mPrev.lock();
}

ConstIClipPtr IClip::getNext() const
{
    return mNext.lock();
}

ConstIClipPtr IClip::getPrev() const
{
    return mPrev.lock();
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void IClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mNext;
    ar & mPrev;
}
template void IClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void IClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


} // namespace