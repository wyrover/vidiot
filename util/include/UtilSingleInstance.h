#ifndef SINGLE_INSTANCE_H
#define SINGLE_INSTANCE_H

#include "UtilLog.h"

/// This template gives a class a CLASS::get() method by which the one and only instantiated object of the class can be obtained.
/// This only works if there's only one instance of the class.
template <class INSTANCETYPE>
class SingleInstance
{
public:
    SingleInstance()
    {
        ASSERT_ZERO(sInstance);
        sInstance = this;
    }
    ~SingleInstance()
    {
        ASSERT_NONZERO(sInstance);
        sInstance = 0;
    }
    static INSTANCETYPE& get()
    {
        return *(static_cast<INSTANCETYPE*>(sInstance));
    }
private:
    static SingleInstance* sInstance;
};

template <class INSTANCETYPE> SingleInstance<INSTANCETYPE>* SingleInstance<INSTANCETYPE>::sInstance = 0;

#endif // SINGLE_INSTANCE_H