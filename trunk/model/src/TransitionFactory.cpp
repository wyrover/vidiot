#include "TransitionFactory.h"

#include "Transition.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionFactory::TransitionFactory(std::string type)
    :   mType(type)
{
}

TransitionFactory::~TransitionFactory()
{
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONS
//////////////////////////////////////////////////////////////////////////

void TransitionFactory::add(TransitionPtr transition)
{
    mTransitions.push_back(transition);
}

TransitionPtr TransitionFactory::getDefault()
{
    ASSERT_NONZERO(mTransitions.size());
    return make_cloned<Transition>(mTransitions.front());
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TransitionFactory& obj )
{
    os << obj.mType << '|' << obj.mTransitions;
    return os;
}

} //namespace