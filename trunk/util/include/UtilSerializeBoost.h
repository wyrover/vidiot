// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef UTIL_SERIALIZE_BOOST_H
#define UTIL_SERIALIZE_BOOST_H

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

namespace boost { namespace serialization {

const std::string sNumerator("numerator");
const std::string sDenominator("denominator");

template<class Archive, class IntType>
void serialize(Archive& ar, boost::rational<IntType>& r, const unsigned int version)
{
    try
    {
        IntType n = 0;
        IntType d = 0;

        if (Archive::is_saving::value)
        {
            n = r.numerator();
            d = r.denominator();
        }

        ar & boost::serialization::make_nvp(sNumerator.c_str(),n) & boost::serialization::make_nvp(sDenominator.c_str(),d);

        if (Archive::is_loading::value)
        {
            r.assign(n,d);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

const std::string sPointer("ptr");

template<class Archive, class TYPE>
void serialize(Archive &ar, boost::weak_ptr<TYPE>& p, const unsigned int version)
{
    try
    {
        if (Archive::is_loading::value)
        {
            boost::shared_ptr<TYPE> shared;
            ar & boost::serialization::make_nvp(sPointer.c_str(), shared);
            p = shared;
        }
        else
        {
            boost::shared_ptr<TYPE> shared = p.lock();
            ar & boost::serialization::make_nvp(sPointer.c_str(), shared);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template<class Archive, class TYPE>
void serialize(Archive &ar, boost::icl::interval_set<TYPE>& set, const unsigned int version)
{
    const std::string sList("listofpairs");
    try
    {
        typedef std::pair<TYPE,TYPE> Pair;
        typedef boost::icl::discrete_interval<TYPE> AnInterval;
        if (Archive::is_loading::value)
        {
            std::list< Pair > list;
            ar & boost::serialization::make_nvp(sList.c_str(),list);
            while (!list.empty())
            {
                Pair a = list.front();
                list.pop_front();
                set += AnInterval(a.first,a.second);
            }
        }
        else
        {
            std::list< Pair > list;
            for ( AnInterval interval : set )
            {
                list.push_back(std::pair<TYPE,TYPE>(interval.lower(),interval.upper()));
            }
            ar & boost::serialization::make_nvp(sList.c_str(),list);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

}} // namespace

#endif
