#ifndef UTIL_SERIALIZE_BOOST_H
#define UTIL_SERIALIZE_BOOST_H

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <boost/filesystem/path.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/rational.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/weak_ptr.hpp>

namespace boost { namespace serialization {

template<class Archive>
void serialize(Archive &ar, boost::filesystem::path &p, const unsigned int version)
{
    std::string s;
    if (Archive::is_saving::value)
    {
        s = p.string();
    }
    ar & s;
    if (Archive::is_loading::value)
    {
        p = s;
    }
}

template<class Archive>
void serialize(Archive& ar, boost::rational<int>& r, const unsigned int version)
{
    int n = 0;
    int d = 0;

    if (Archive::is_saving::value)
    {
        n = r.numerator();
        d = r.denominator();
    }

    ar & n & d;

    if (Archive::is_loading::value)
    {
        r.assign(n,d);
    }
}

template<class Archive, class TYPE>
void serialize(Archive &ar, boost::weak_ptr<TYPE>& p, const unsigned int version)
{
    if (Archive::is_loading::value)
    {
        boost::shared_ptr<TYPE> shared;
        ar & shared;
        p = shared;
    }
    else
    {
        ar & p.lock();
    }
}

template<class Archive, class TYPE>
void serialize(Archive &ar, boost::icl::interval_set<TYPE>& set, const unsigned int version)
{
    typedef std::pair<TYPE,TYPE> Pair;
    typedef boost::icl::discrete_interval<TYPE> AnInterval;
    if (Archive::is_loading::value)
    {
        std::list< Pair > list;
        ar & list;
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
        BOOST_FOREACH( AnInterval interval, set )
        {
            list.push_back(std::pair<TYPE,TYPE>(interval.lower(),interval.upper()));
        }
        ar & list;
    }
}

}} // namespace

#endif //UTIL_SERIALIZE_BOOST_H