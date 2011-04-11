#ifndef GUI_DATA_OBJECT_H
#define GUI_DATA_OBJECT_H

#include <wx/dataobj.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include "AProjectViewNode.h"

namespace gui {

class GuiDataObject
    :   public wxDataObjectSimple
    ,   public boost::noncopyable
{
public:

    /// A callback method which is called upon destruction.
    /// This is used to let the ProjectView know the end of a drag operation.
    typedef boost::function<void ()> CallbackOnDestruction;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    GuiDataObject();
    GuiDataObject(model::ProjectViewPtrs assets, CallbackOnDestruction callback = 0);
    ~GuiDataObject();

    static const wxString sFormat;

    //////////////////////////////////////////////////////////////////////////
    // FROM wxDataObjectSimple
    //////////////////////////////////////////////////////////////////////////

    virtual bool GetDataHere(void *buf) const;
    virtual size_t GetDataSize () const;
    virtual bool SetData(size_t len, const void *buf);

    //////////////////////////////////////////////////////////////////////////
    // GET ALL ASSETS
    //////////////////////////////////////////////////////////////////////////

    model::ProjectViewPtrs getAssets() const;

private:

    wxDataFormat mFormat;
    model::ProjectViewPtrs mAssets;
    CallbackOnDestruction mCallback;
};

} // namespace

#endif // GUI_DATA_OBJECT_H