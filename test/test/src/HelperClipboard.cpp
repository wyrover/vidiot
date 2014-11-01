// Copyright 2014 Eric Raijmakers.
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

#include "Test.h"

namespace test {


struct DummyDataObject : public wxDataObjectSimple
{
    DummyDataObject()
        : wxDataObjectSimple("DummyDataObject")
    {
    }
    bool GetDataHere(void *buf) const override
    {
        return true;
    }
    virtual size_t GetDataSize() const override
    {
        return 0;
    }
};

void ClearClipboard()
{
    LOG_ERROR;
    bool ok = wxTheClipboard->Open();
    ASSERT(ok);
    wxTheClipboard->SetData(new DummyDataObject());
    // NOT: wxTheClipboard->Clear(); -- does not work
    wxTheClipboard->Close();
    LOG_ERROR;
    ASSERT_CLIPBOARD_EMPTY();
}

void AssertClipboardEmpty()
{
    LOG_ERROR;
    bool ok = wxTheClipboard->Open();
    ASSERT(ok);
    ASSERT(!wxTheClipboard->IsSupported(wxDataFormat(gui::ProjectViewDataObject::sFormat)));
    ASSERT(!wxTheClipboard->IsSupported(wxDataFormat(gui::timeline::TimelineDataObject::sFormat)));
    ASSERT(!wxTheClipboard->IsSupported(wxDataFormat(wxDF_FILENAME)));
    wxTheClipboard->Close();
    LOG_ERROR;
}

void AssertClipboardContains(const wxDataFormat& format)
{
    LOG_ERROR;
    bool ok = wxTheClipboard->Open();
    ASSERT(ok);
    ASSERT(wxTheClipboard->IsSupported(format));
    wxTheClipboard->Close();
    LOG_ERROR;
}

void FillClipboardWithFiles(std::list<wxFileName> files)
{
    wxFileDataObject* data = new wxFileDataObject();
    for (wxFileName file : files)
    {
        data->AddFile(file.GetFullPath());
    }
    LOG_ERROR;
    bool ok = wxTheClipboard->Open();
    ASSERT(ok);
    wxTheClipboard->SetData(data);
    wxTheClipboard->Close();
    LOG_ERROR;
}


} // namespace
