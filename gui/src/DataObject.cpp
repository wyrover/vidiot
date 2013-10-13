// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "DataObject.h"

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
    for ( model::NodePtr asset : mAssets )
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
    ASSERT_ZERO(len % sizeof(model::NodeId));

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