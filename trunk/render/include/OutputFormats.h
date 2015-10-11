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

#pragma once

namespace model { namespace render {

class OutputFormats
{
public:
    static void initialize();
    static void add(const OutputFormat& format);

    static OutputFormatList getList();

    static wxStrings getNames();
    static OutputFormatPtr getByName(const wxString& name);  ///< \return cloned default output format
    static OutputFormatPtr getByExtension(const wxString& extension); ///< \return cloned output format to be used for given extension

    static OutputFormatPtr getDefault();

private:
    static OutputFormatList sOutputFormats;
};

}} // namespace
