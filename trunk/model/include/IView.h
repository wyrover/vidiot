#ifndef MODEL_I_VIEW_H
#define MODEL_I_VIEW_H


namespace model {

class IView
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual ~IView() {};

    static IView& getView();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    /// Used for submitting project level events. These cannot be submitted by
    /// the project itselves since it involves creation/destruction of that project
    virtual void ProcessModelEvent( wxEvent& event ) = 0;

    /// Used for submitting project level events. These cannot be submitted by
    /// the project itselves since it involves creation/destruction of that project
    virtual void QueueModelEvent( wxEvent* event ) = 0;

private:

    ////////////////////////////////////////////////////////////////////////////
    //// SERIALIZATION
    ////////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::IView, 1)

#endif // MODEL_I_VIEW_H