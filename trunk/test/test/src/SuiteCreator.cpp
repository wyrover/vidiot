#pragma warning(disable:4996)

#include "SuiteCreator.h"

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include "Window.h"

namespace test {

wxString sSuite;
boost::optional<std::string> sTest(boost::none);
int sSuiteCount = 1;

void updateTitle()
{
    wxString s;
    s << sSuiteCount << ": " << sSuite;
    if (sTest)
    {
        s << ": " << *sTest;
    }
    gui::Window::get().setAdditionalTitle(s);
}

void setSuite(const char* suite)
{
    std::vector<std::string> strs;
    boost::split(strs, suite, boost::is_any_of(":"));
    sSuite = strs.back();
    sSuite.Replace("test","Test",false);
    sTest.reset();
    updateTitle();
    sSuiteCount++;
}

void setTest(const char* test)
{
    sTest.reset(test);
    updateTitle();
}

} // namespace