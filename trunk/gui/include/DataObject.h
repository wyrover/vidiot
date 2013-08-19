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

#ifndef DATA_OBJECT_H
#define DATA_OBJECT_H

namespace gui {

class DataObject
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

    DataObject();
    DataObject(model::NodePtrs assets, CallbackOnDestruction callback = 0);
    virtual ~DataObject();

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

    model::NodePtrs getAssets() const;

private:

    wxDataFormat mFormat;
    model::NodePtrs mAssets;
    CallbackOnDestruction mCallback;
};

} // namespace

#endif // DATA_OBJECT_H