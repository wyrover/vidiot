#ifndef UTIL_LOG_GENERIC_H
#define UTIL_LOG_GENERIC_H

#include <list>
#include <map>
#include <ostream>
#include <boost/foreach.hpp>

class ILoggable
{
public:
    virtual std::string toString() const = 0;
};

std::ostream& operator<< (std::ostream& os, const ILoggable& obj)
{
    os << obj.toString();
    return os;
}

template <class T>
std::ostream& operator<< (std::ostream& os, const std::list<T>& obj)
{
    os << "{ ";
    BOOST_FOREACH( T child, obj )
    {
        os << child << " ";
    }
    os << "}";
    return os;
}

template <class T>
std::ostream& operator<< (std::ostream& os, const std::vector<T>& obj)
{
    os << "{ ";
    BOOST_FOREACH( T child, obj )
    {
        os << child << " ";
    }
    os << "}";
    return os;
}

template <class T, class U>
std::ostream& operator<< (std::ostream& os, const std::pair<T,U>& obj)
{
    os << "("<< obj.first << "," << obj.second << ")";
    return os;
}

#endif //UTIL_LOG_GENERIC_H

