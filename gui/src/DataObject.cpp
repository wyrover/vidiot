#include "DataObject.h"

#include <boost/foreach.hpp>
#include "Node.h"
#include "UtilLog.h"

namespace gui {

const wxString DataObject::sFormat = wxString("application/vidiot");

DataObject::DataObject()
:   wxDataObjectSimple()
,   mFormat(sFormat)
,   mAssets()
,   mCallback(0)
{
    SetFormat(mFormat);
}


DataObject::DataObject(model::NodePtrs assets, CallbackOnDestruction callback)
:   wxDataObjectSimple()
,   mFormat(sFormat)
,   mAssets(assets)
,   mCallback(callback)
{
    SetFormat(mFormat);
}

DataObject::~DataObject()
{
    if (mCallback)
    {
        mCallback();
    }
}

//////////////////////////////////////////////////////////////////////////
// FROM wxDataObjectSimple
//////////////////////////////////////////////////////////////////////////

bool DataObject::GetDataHere(void *buf) const
{
    unsigned int i = 0;
    BOOST_FOREACH( model::NodePtr asset, mAssets )
    {
        static_cast<model::NodeId*>(buf)[i] = asset->id();
        i++;
    }
    return true;
}

size_t DataObject::GetDataSize () const
{
    return sizeof(model::NodeId) * mAssets.size();
}

bool DataObject::SetData(size_t len, const void *buf)
{
    ASSERT(len % sizeof(model::NodeId) == 0)(len);

    mAssets.clear();

    model::NodeId* index = static_cast<model::NodeId*>(const_cast<void*>(buf));
    for (unsigned int i = 0; i < len / sizeof(model::NodeId); ++i)
    {
        mAssets.push_back(model::Node::Ptr(static_cast<const model::NodeId*>(buf)[i]));
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET ALL ASSETS
//////////////////////////////////////////////////////////////////////////

model::NodePtrs DataObject::getAssets() const
{
    return mAssets;
}

} // namespace
