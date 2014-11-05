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

#ifndef PROJECT_VIEW_DATA_OBJECT_H
#define PROJECT_VIEW_DATA_OBJECT_H

namespace gui {

class ProjectViewDataObject
    :   public wxDataObjectSimple
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
    // FROM wxDataObjectSimple
    //////////////////////////////////////////////////////////////////////////

    virtual bool GetDataHere(void *buf) const override;
    virtual size_t GetDataSize () const override;
    virtual bool SetData(size_t len, const void *buf) override;

    //////////////////////////////////////////////////////////////////////////
    // GET ALL ASSETS
    //////////////////////////////////////////////////////////////////////////

	bool checkIfOkForPasteOrDrop() const;

    model::NodePtrs getAssets() const; // todo rename to getNodes

private:

    wxDataFormat mFormat;
    model::NodePtrs mAssets;
};

} // namespace

#endif