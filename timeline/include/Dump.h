#ifndef DUMP_H
#define DUMP_H

#include "Part.h"

namespace gui { namespace timeline {

class Dump
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Dump(Timeline* timeline);
    virtual ~Dump();

    //////////////////////////////////////////////////////////////////////////
    // DEBUG
    //////////////////////////////////////////////////////////////////////////

    void dump();
};

}} // namespace

#endif // DUMP_H