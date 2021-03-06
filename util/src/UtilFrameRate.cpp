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

#include "UtilFrameRate.h"
#include "UtilSerializeBoost.h"

const FrameRate FrameRate::s24p = FrameRate(24 * 1000, 1001);
const FrameRate FrameRate::s25p = FrameRate(       25, 1);
const FrameRate FrameRate::s30p = FrameRate(30 * 1000, 1001);

typedef boost::tuple<wxString, FrameRate> FrameRateEntry;
const std::vector<FrameRateEntry> sPossibleFrameRates = {
    { wxT("23.97"), FrameRate::s24p },
    { wxT("25"), FrameRate::s25p },
    { wxT("29.97"), FrameRate::s30p },
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FrameRate::FrameRate(const int64_t& num, const int64_t& den)
    : rational64(num,den)
{
}

FrameRate::FrameRate(const wxString& framerate)
    : rational64(FrameRate::s24p) // Default value. Only used if the given framerate is unknown.
{
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (framerate.IsSameAs(boost::get<0>(sPossibleFrameRates[i])))
        {
            assign(boost::get<1>(sPossibleFrameRates[i]).numerator(), boost::get<1>(sPossibleFrameRates[i]).denominator());
            break;
        }
    };
};

FrameRate::FrameRate(const rational64& r)
    : rational64(r.numerator(),r.denominator())
{
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

// static
std::vector<FrameRate> FrameRate::getSupported()
{
    std::vector<FrameRate> result;
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        result.emplace_back(boost::get<1>(sPossibleFrameRates[i]));
    };
    return result;
}

wxString FrameRate::toString() const
{
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (*this == boost::get<1>(sPossibleFrameRates[i]))
        {
            return boost::get<0>(sPossibleFrameRates[i]);
        }
    };
    wxString result; result << numerator() << "/" << denominator();
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const FrameRate& obj)
{
    os << obj.numerator() << '/' << obj.denominator();
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void FrameRate::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & boost::serialization::make_nvp("rational64", boost::serialization::base_object< rational64 >(*this));
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void FrameRate::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void FrameRate::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);