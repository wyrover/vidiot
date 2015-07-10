// Copyright 2015 Eric Raijmakers.
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

#include "AudioPeaks.h"

#include <boost/serialization/binary_object.hpp>

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioPeaks::AudioPeaks()
    : std::vector< AudioPeak >()
{}

AudioPeaks::AudioPeaks(AudioPeaks::const_iterator b, AudioPeaks::const_iterator e)
    : std::vector< AudioPeak >(b, e)
{}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

const std::string sCount("nPeaks");
const std::string sPeaks("peaks");

template<class Archive>
void AudioPeaks::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        if (Archive::is_loading::value)
        {
            unsigned int s{ 0 };
            ar & boost::serialization::make_nvp(sCount.c_str(), s);
            resize(s);
            ar & boost::serialization::make_nvp(sPeaks.c_str(), boost::serialization::make_binary_object(data(), s * sizeof(AudioPeak)));
        }
        else
        {
            unsigned int s{ static_cast<unsigned int>(size()) };
            ar & boost::serialization::make_nvp(sCount.c_str(), s);
            ar & boost::serialization::make_nvp(sPeaks.c_str(), boost::serialization::make_binary_object(data(), s * sizeof(AudioPeak)));
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void AudioPeaks::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioPeaks::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace
