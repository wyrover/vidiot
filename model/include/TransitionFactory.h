#ifndef MODEL_TRANSITION_FACTORY_H
#define MODEL_TRANSITION_FACTORY_H

namespace model {

class TransitionFactory
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TransitionFactory(std::string type);

    virtual ~TransitionFactory();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONS
    //////////////////////////////////////////////////////////////////////////

    void add(TransitionPtr transition);
    TransitionPtr getDefault();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    std::string mType;
    std::list< TransitionPtr > mTransitions;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const TransitionFactory& obj );
};

} // namespace

#endif // MODEL_TRANSITION_FACTORY_H