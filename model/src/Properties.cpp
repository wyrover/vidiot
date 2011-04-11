#include "Properties.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "UtilSerializeBoost.h"
#include "Options.h"

namespace model {

Properties::Properties()
:   mFrameRate(gui::Options::getDefaultFrameRate())
{
}

Properties::~Properties()
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

FrameRate Properties::getFrameRate()
{
    return mFrameRate;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Properties::serialize(Archive & ar, const unsigned int version)
{
    ar & mFrameRate;
}
template void Properties::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Properties::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
