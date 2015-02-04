// Copyright 2013-2015 Eric Raijmakers.
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
:   wxTextDataObject()
,   mAssets()
{
    SetFormat(wxDataFormat(sFormat));
    wxTextDataObject::SetText(serialize());
    VAR_ERROR(GetText().Length());
}

ProjectViewDataObject::ProjectViewDataObject(const model::NodePtrs& assets)
:   wxTextDataObject()
,   mAssets(assets)
{
    SetFormat(wxDataFormat(sFormat));
    wxTextDataObject::SetText(serialize());
    VAR_ERROR(GetText().Length());
}

ProjectViewDataObject::~ProjectViewDataObject()
{
}

//////////////////////////////////////////////////////////////////////////
// FROM wxTextDataObject
//////////////////////////////////////////////////////////////////////////

//bool ProjectViewDataObject::SetData(size_t len, const void *buf)
//{
//    bool ok = wxTextDataObject::SetData(len, buf);
//    if (ok)
//    {
//        VAR_ERROR(GetText().Length());
//        deserialize(GetText());
//    }
//    return ok;
//}

void ProjectViewDataObject::SetText(const wxString& text)
{
    wxTextDataObject::SetText(text);
    deserialize(GetText());
}

wxDataFormat ProjectViewDataObject::GetPreferredFormat(Direction dir) const
{
    return wxDataFormat(sFormat);
}

//size_t ProjectViewDataObject::GetFormatCount(wxDataObject::Direction dir) const
//{
//    return wxTextDataObject::GetFormatCount(dir);
//    return 1;
//}
//
//void ProjectViewDataObject::GetAllFormats(wxDataFormat *formats, wxDataObject::Direction dir) const
//{
//    wxTextDataObject::GetAllFormats(formats,dir);
//    //wxDataFormat* format = new wxDataFormat(sFormat);
//    //formats = format;
//}

//////////////////////////////////////////////////////////////////////////
// GET ALL ASSETS
//////////////////////////////////////////////////////////////////////////

model::NodePtrs ProjectViewDataObject::getNodes() const
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

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

const std::string sXmlName("nodes");

void ProjectViewDataObject::deserialize(wxString from)
{
    VAR_ERROR(from.Length());
    std::istringstream store(from.ToStdString());
    try
    {
        boost::archive::xml_iarchive ar(store);
        ar & boost::serialization::make_nvp(sXmlName.c_str(), *this);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

wxString ProjectViewDataObject::serialize() const
{
    std::ostringstream store;
    try
    {
        boost::archive::xml_oarchive ar(store);
        ar & boost::serialization::make_nvp(sXmlName.c_str(), *this);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
    VAR_ERROR(store.str().size());
    return store.str();
}


template<class Archive>
void ProjectViewDataObject::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(mAssets);
    VAR_ERROR(mAssets);
}

template void ProjectViewDataObject::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ProjectViewDataObject::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace
