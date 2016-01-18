// Copyright 2014-2016 Eric Raijmakers.
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

#include "WximageClip.h"

#include "Config.h"
#include "WximageFile.h"
#include "VideoCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WximageClip::WximageClip()
    : VideoClip()
{
    VAR_DEBUG(*this);
}

WximageClip::WximageClip(const WximageFilePtr& file)
    : VideoClip(file)
{
    VAR_DEBUG(*this);

    pts length = getLength();
    pts half = length / 2;
    pts remainingLength = Config::get().ReadLong(Config::sPathTimelineDefaultStillImageLength);

    // Move right edge to the left such that the clip can be extended if required
    adjustEnd( - half );

    // Move left edge to the right sich that the clip can be extended if required. The -remainingLength ensures that the resulting clip has the correct resulting size
    adjustBegin(getLength() - remainingLength); // do not replace 'getLength()' with 'half' here (try doing the computation with 'length' an odd number...)

    ASSERT_EQUALS(getLength(),remainingLength);
}

WximageClip::~WximageClip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const WximageClip& obj)
{
    os << static_cast<const VideoClip&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WximageClip::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoClip);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WximageClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WximageClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::WximageClip)