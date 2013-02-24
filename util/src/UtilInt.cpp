#include "UtilInt.h"

samplecount min(samplecount first, samplecount second)
{
    return first < second ? first : second;
}

int floor(boost::rational<int> r)
{
    return static_cast<int>(floor(boost::rational_cast<double>(r)));
}