#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <wx/app.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "ModelPtr.h"
#include "ProjectEventOpenProject.h"
#include "ProjectEventCloseProject.h"

//#define CATCH_ALL_ERRORS

class GuiWindow;

class GuiMain : public wxApp
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    GuiMain();
    ~GuiMain();

    //////////////////////////////////////////////////////////////////////////
    // PROJECTS 
    //////////////////////////////////////////////////////////////////////////

    void OnOpenProject( ProjectEventOpenProject &event );
    void OnCloseProject( ProjectEventCloseProject &event );

    model::Project* getProject() const;

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

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnIdle(wxIdleEvent& event);

private:

    model::Project* mProject;
    bool mDone;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

DECLARE_APP(GuiMain)

#endif // GUI_MAIN_H
