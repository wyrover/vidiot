#pragma warning(disable:4996)

#include "SuiteCreator.h"

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include "Window.h"

namespace test {

std::string sSuite("");
boost::optional<std::string> sTest(boost::none);

void updateTitle()
{
    static int testcount = 1;
    wxString s;
    s << testcount << " - " << sSuite;
    if (sTest)
    {
        s << " - " << *sTest;
    }
    gui::Window::get().setAdditionalTitle(s);
    testcount++;
}

void setSuite(const char* suite)
{
    std::vector<std::string> strs;
    boost::split(strs, suite, boost::is_any_of(":"));
    sSuite = strs.back();
    sTest.reset();
    updateTitle();
}

void setTest(const char* test)
{
    sTest.reset(test);
    updateTitle();
}

} // namespace