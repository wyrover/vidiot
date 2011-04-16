#ifndef UTIL_SERIALIZE_BOOST_H
#define UTIL_SERIALIZE_BOOST_H

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <boost/rational.hpp>
#include <boost/filesystem/path.hpp>

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

}} // namespace

#endif //UTIL_SERIALIZE_BOOST_H

