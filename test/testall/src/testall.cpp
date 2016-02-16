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
#include <wx/filename.h>
#include <sstream>
#include <iostream>
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
    std::vector<wxString> tests{ "testauto", "testfiletypes", "testtransitions", "testsync", "testui", "testrendering" };
    wxApp::SetInstance(new wxApp());
    wxEntryStart(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#else
int main(int argc, char *argv[])
{
    std::vector<wxString> tests{ "testauto", "testfiletypes", "testtransitions", "testsync", "testui", "testrendering" };
    wxApp::SetInstance(new wxApp());
    wxEntryStart(argc,argv);

#endif // _MSC_VER

#ifdef _DEBUG
    wxString sVersion("Debug");
#else
    wxString sVersion("RelWithDebInfo");
#endif

    long starttime = time(0);
    long returnvalue{ 0 };

    for (wxString test : tests)
    {
        wxMessageOutputDebug().Output(test.Capitalize() + " ...");
        long teststarttime = time(0);
        wxFileName path{ wxFileName(wxGetCwd(), "") };
        path.SetName(test);

        path.AppendDir("..");
        path.AppendDir(test);
#ifdef _MSC_VER
        path.AppendDir(sVersion);
        path.SetExt("exe");
#else
        path.SetExt("run");
#endif // _MSC_VER
        returnvalue = wxExecute(path.GetFullPath(), wxEXEC_SYNC);
        if (returnvalue != 0) { break; }
        wxMessageOutputDebug().Output(test.Capitalize() + " done " + getTimes(teststarttime));
    }

    wxMessageOutputDebug().Output("Total running time: " + getTimes(starttime));
    wxEntryCleanup();
#ifdef _MSC_VER
    SetThreadExecutionState(ES_CONTINUOUS); // Enable sleep again
#endif // _MSC_VER
    return returnvalue;
}

