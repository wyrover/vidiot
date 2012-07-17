#ifndef UTIL_CLONEABLE_H
#define UTIL_CLONEABLE_H

#include "UtilRTTI.h"

/// Needed for cloning in class hierarchies starting with abstract base classes.
/// Use the method make_cloned to make copies of objects
class ICloneable
    :   public IRTTI
{
public:
    virtual ~ICloneable() {};
    virtual ICloneable* clone() const = 0;
};

template <typename T>
boost::shared_ptr<T> make_cloned_ptr(T& t)
{
    return boost::shared_ptr<T>(t.clone());
}

template <typename T>
boost::shared_ptr<T> make_cloned(boost::shared_ptr<T> t)
{
    return boost::shared_ptr<T>(t->clone());
}

template <typename T>
std::list< boost::shared_ptr<T> > make_cloned(std::list< boost::shared_ptr<T> > elements)
{
    std::list< boost::shared_ptr<T> > result;
    BOOST_FOREACH( boost::shared_ptr<T> element, elements )
    {
        result.push_back(make_cloned<T>(element));
    }
    return result;
}

#endif // UTIL_CLONEABLE_H