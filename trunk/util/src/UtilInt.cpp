#include "UtilInt.h"

pts abs(pts p)
{ 
    return p >= 0 ? p : -p; 
}

samplecount min(samplecount first, samplecount second)
{
    return first < second ? first : second;
}


