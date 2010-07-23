#include "GuiDataObject.h"

#include <boost/foreach.hpp>
#include "AProjectViewNode.h"
#include "UtilLog.h"

namespace gui {

const wxString GuiDataObject::sFormat = wxString("application/vidiot");

GuiDataObject::GuiDataObject()
:   wxDataObjectSimple()
,   mFormat(sFormat)
,   mAssets()
{
    SetFormat(mFormat);
}

GuiDataObject::GuiDataObject(model::ProjectViewPtrs assets)
:   wxDataObjectSimple()
,   mFormat(sFormat)
,   mAssets(assets)
{
    SetFormat(mFormat);
}

GuiDataObject::~GuiDataObject()
{
}

//////////////////////////////////////////////////////////////////////////
// FROM wxDataObjectSimple
//////////////////////////////////////////////////////////////////////////

bool GuiDataObject::GetDataHere(void *buf) const
{
    unsigned int i = 0;
    BOOST_FOREACH(model::ProjectViewPtr asset, mAssets)
    {
        static_cast<model::ProjectViewId*>(buf)[i] = asset->id();
        i++;
    }
    return true;
}

size_t GuiDataObject::GetDataSize () const
{
    return sizeof(model::ProjectViewId) * mAssets.size();
}

bool GuiDataObject::SetData(size_t len, const void *buf)
{
    ASSERT(len % sizeof(model::ProjectViewId) == 0)(len);

    mAssets.clear();

    model::ProjectViewId* index = static_cast<model::ProjectViewId*>(const_cast<void*>(buf));
    for (unsigned int i = 0; i < len / sizeof(model::ProjectViewId); ++i)
    {
        mAssets.push_back(model::AProjectViewNode::Ptr(static_cast<const model::ProjectViewId*>(buf)[i]));
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET ALL ASSETS
//////////////////////////////////////////////////////////////////////////

model::ProjectViewPtrs GuiDataObject::getAssets() const
{
    return mAssets;
}

} // namespace
