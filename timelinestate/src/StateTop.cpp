#include "StateTop.h"

#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateTop::StateTop( my_context ctx ) // entry
    :   TimeLineStateInner( ctx )
{
    LOG_DEBUG; 
}

StateTop::~StateTop() // exit
{ 
    LOG_DEBUG; 
}

}}} // namespace

