#ifndef APPLICATION_H
#define APPLICATION_H

#include <wx/app.h>

#ifdef _MSC_VER
//#define CATCH_ALL_ERRORS
#endif // _MSC_VER

namespace gui {

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
};

} // namespace

#endif // APPLICATION_H
