#ifndef GUI_DATA_OBJECT_H
#define GUI_DATA_OBJECT_H

#include <wx/dataobj.h>
#include "AProjectViewNode.h"

class GuiDataObject : public wxDataObjectSimple
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    GuiDataObject();
    GuiDataObject(model::ProjectViewPtrs assets);
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
};

#endif // GUI_DATA_OBJECT_H