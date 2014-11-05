// Copyright 2013,2014 Eric Raijmakers.
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

#include "ProjectViewDataObject.h"

#include "Folder.h"
#include "Node.h"
#include "Sequence.h"
#include "UtilLog.h"

namespace gui {

const wxString ProjectViewDataObject::sFormat = wxString("application/vidiot/nodes");

ProjectViewDataObject::ProjectViewDataObject()
:   wxDataObjectSimple()
,   mFormat(sFormat)
,   mAssets()
{
    SetFormat(mFormat);
}

ProjectViewDataObject::ProjectViewDataObject(const model::NodePtrs& assets)
:   wxDataObjectSimple()
,   mFormat(sFormat)
,   mAssets(assets)
{
    SetFormat(mFormat);
}

ProjectViewDataObject::~ProjectViewDataObject()
{
}

//////////////////////////////////////////////////////////////////////////
// FROM wxDataObjectSimple
//////////////////////////////////////////////////////////////////////////

bool ProjectViewDataObject::GetDataHere(void *buf) const
{
    unsigned int i = 0;
    for ( model::NodePtr asset : mAssets )
    {
        static_cast<model::NodeId*>(buf)[i] = asset->id();
        i++;
    }
    return true;
}

size_t ProjectViewDataObject::GetDataSize () const
{
    return sizeof(model::NodeId) * mAssets.size();
}

bool ProjectViewDataObject::SetData(size_t len, const void *buf)
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

model::NodePtrs ProjectViewDataObject::getAssets() const
{
    return mAssets;
}

bool ProjectViewDataObject::checkIfOkForPasteOrDrop() const
{
	if (mAssets.empty())
	{
		return false;
	}
	for (model::NodePtr node : mAssets)
	{
		if (node->isA<model::Sequence>())
		{
			gui::StatusBar::get().timedInfoText(_("Sequences can not be pasted/dropped."));
			return false;
		}
		else if (node->isA<model::Folder>())
		{
			gui::StatusBar::get().timedInfoText(_("Folders can not be pasted/dropped."));
			return false;
		}
	}
	return true;
}


} // namespace