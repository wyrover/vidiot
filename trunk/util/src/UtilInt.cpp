#include "UtilInt.h"

pts abs(pts p)
{ 
    return p >= 0 ? p : -p; 
}

samples_t min(samples_t first, samples_t second)
{
    return first < second ? first : second;
}


