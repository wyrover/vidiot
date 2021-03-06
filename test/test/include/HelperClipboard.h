// Copyright 2014-2016 Eric Raijmakers.
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

#pragma once

#include "Test.h"

namespace test {

void ClearClipboard();

void AssertClipboardEmpty();

void AssertClipboardContains(const wxDataFormat& format);

void FillClipboardWithFiles(wxFileNames files);

#define ASSERT_CLIPBOARD_EMPTY AssertClipboardEmpty
#define ASSERT_CLIPBOARD_CONTAINS_NODES AssertClipboardContains(wxDataFormat(gui::ProjectViewDataObject::sFormat))
#define ASSERT_CLIPBOARD_CONTAINS_CLIPS AssertClipboardContains(wxDataFormat(gui::timeline::TimelineDataObject::sFormat))
#define ASSERT_CLIPBOARD_CONTAINS_FILES AssertClipboardContains(wxDataFormat(wxDF_FILENAME))

} // namespace
