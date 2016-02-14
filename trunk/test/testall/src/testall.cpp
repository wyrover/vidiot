// Copyright 2016 Eric Raijmakers.
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

#include <wx/app.h>
#include <sstream>
#include <iomanip>

wxString getTimes(long start)
{
    long t = time(0) - start;
    std::ostringstream os;
    os << (t / 60) << "m" << std::setw(2) << std::setfill('0') << (t % 60);
    return os.str();
}

#ifdef _MSC_VER

#include <windows.h>

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow)
{
     SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED); // Avoid sleep
    long starttime = time(0);
    long returnvalue{ 0 };
    wxApp::SetInstance(new wxApp());
    wxEntryStart(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#ifdef _DEBUG
    wxString sVersion("Debug");
#else
    wxString sVersion("RelWithDebInfo");
#endif

    for (wxString test : { "testauto", "testfiletypes", "testtransitions", "testsync", "testui", "testrendering" })
    {
        wxMessageOutputDebug().Output(test.Capitalize() + " ...");
        long teststarttime = time(0);
        returnvalue = wxExecute(wxGetCwd() + "\\..\\" + test + "\\" + sVersion + "\\" + test + ".exe", wxEXEC_SYNC);
        if (returnvalue != 0) { break; }
        wxMessageOutputDebug().Output(test.Capitalize() + " done " + getTimes(teststarttime));
    }

    wxMessageOutputDebug().Output("Total running time: " + getTimes(starttime));
    wxEntryCleanup();
    SetThreadExecutionState(ES_CONTINUOUS); // Enable sleep again
    return returnvalue;
}

#else

int main(int argc, char *argv[])
{
    return 0;
}

#endif // _MSC_VER