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

#include "UtilStackWalker.h"

#include "UtilLog.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StackWalker::StackWalker()
{
    mOverview << std::endl;
}

StackWalker::~StackWalker()
{
}

std::string StackWalker::show()
{
    Walk();
    return mOverview.str();
}

//////////////////////////////////////////////////////////////////////////
// THE INTERFACE
//////////////////////////////////////////////////////////////////////////

void StackWalker::OnStackFrame(const wxStackFrame &frame)
{
    mOverview
        << std::setw(3) << std::setfill(' ') << frame.GetLevel() << ' '
        << frame.GetAddress() << ' '
        << std::setw(8) << std::setfill(' ') << frame.GetOffset() << ' '
        << frame.GetModule() << ' ';
    if (frame.HasSourceLocation())
    {
        mOverview << frame.GetFileName() << '(' << frame.GetLine() << "): ";
    }
    mOverview  << frame.GetName();
    mOverview << std::endl;

    //mOverview << '(';
    //for (size_t i = 0; i < frame.GetParamCount(); ++i)
    //{
    //    wxString type;
    //    wxString name;
    //    wxString value;
    //    frame.GetParam(i,&type,&name,&value);
    //    if (i > 0)
    //    {
    //        mOverview << ',';
    //    }
    //    mOverview << type;
    //}
    //mOverview << ')';
}