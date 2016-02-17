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

#ifdef _MSC_VER
#include <windows.h>
void no_sleep_allowed() { SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED); };
void sleep_allowed() {  SetThreadExecutionState(ES_CONTINUOUS); }
void log(const wxString& msg) { std::cout << msg << std::endl; wxMessageOutputDebug().Output(msg); }
wxString get_ext() { return "exe"; }
std::vector<wxString> get_tests() { return { "testauto", "testfiletypes", "testtransitions", "testsync", "testui", "testrendering" }; };
#else
void no_sleep_allowed() {};
void sleep_allowed() {}
void log(const wxString& msg) { std::cout << msg << std::endl; }
wxString get_ext() { return "run"; }
std::vector<wxString> get_tests() { return { "testauto", "testfiletypes", "testtransitions", "testsync", "testrendering", "testui" }; };
#endif

wxString getTimes(long start)
{
    long t = time(0) - start;
    std::ostringstream os;
    os << (t / 60) << "m" << std::setw(2) << std::setfill('0') << (t % 60);
    return os.str();
}

int main(int argc, char *argv[])
{
    no_sleep_allowed();
    wxApp::SetInstance(new wxAppConsole());
    wxEntryStart(argc,argv);

    long starttime = time(0);
    long returnvalue{ 0 };

    for (wxString test : get_tests())
    {
        log(test.Capitalize() + " ...");
        long teststarttime = time(0);
        wxFileName path{ wxFileName(wxGetCwd(), "") };
        path.SetName(test);
        path.SetExt(get_ext());

        path.AppendDir("..");
        path.AppendDir(test);
#ifdef _MSC_VER
#ifdef _DEBUG
        path.AppendDir("Debug");
#else
        path.AppendDir("RelWithDebInfo");
#endif
#endif // _MSC_VER
        returnvalue = wxExecute(path.GetFullPath(), wxEXEC_SYNC);
        if (returnvalue != 0)
        {
            log(test.Capitalize() + " FAILED");
            break;
        }
        log(test.Capitalize() + " done " + getTimes(teststarttime));
    }

    log("Total running time: " + getTimes(starttime));
    wxEntryCleanup();
    sleep_allowed();
#ifdef _MSC_VER
    log("Grep for \"Detected memory leaks\" in Output pane.");
#endif // _MSC_VER
    std::cin.ignore();
    return returnvalue;
}
