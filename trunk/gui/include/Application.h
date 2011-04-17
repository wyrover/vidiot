#ifndef MAIN_H
#define MAIN_H

#include <wx/app.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>

#ifdef _MSC_VER
//#define CATCH_ALL_ERRORS
#endif // _MSC_VER

namespace gui {

class Window;
class Main
    :   public wxApp
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Main();
    ~Main();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    bool OnInit();
    int OnRun();
    int OnExit();

#ifdef CATCH_ALL_ERRORS
    virtual void OnAssertFailure(const wxChar *file, int Line, const wxChar *func, const wxChar *cond, const wxChar *msg);
    virtual bool OnExceptionInMainLoop();
    virtual void OnUnhandledException();
    virtual void OnFatalException();
#endif //CATCH_ALL_ERRORS

private:

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

DECLARE_APP(Main)

} // namespace

#endif // MAIN_H
