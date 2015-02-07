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

#ifndef PROJECT_VIEW_DATA_OBJECT_H
#define PROJECT_VIEW_DATA_OBJECT_H

namespace gui {

class ProjectViewDataObject
    :   public wxTextDataObject
    ,   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectViewDataObject();
    ProjectViewDataObject(const model::NodePtrs& assets);
    virtual ~ProjectViewDataObject();

    static const wxString sFormat;

    //////////////////////////////////////////////////////////////////////////
    // FROM wxTextDataObject
    //////////////////////////////////////////////////////////////////////////

    virtual void SetText(const wxString& text) override;

    wxDataFormat GetPreferredFormat(Direction dir=Get) const override;

    //////////////////////////////////////////////////////////////////////////
    // GET ALL ASSETS
    //////////////////////////////////////////////////////////////////////////

	bool checkIfOkForPasteOrDrop() const;

    model::NodePtrs getNodes() const;

private:

	//////////////////////////////////////////////////////////////////////////
	// MEMBERS
	//////////////////////////////////////////////////////////////////////////

	model::NodePtrs mAssets;

	//////////////////////////////////////////////////////////////////////////
	// SERIALIZATION
	//////////////////////////////////////////////////////////////////////////

	void deserialize(wxString from);
	wxString serialize() const;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

} // namespace

#endif
