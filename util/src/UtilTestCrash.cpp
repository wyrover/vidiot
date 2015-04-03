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

#include "UtilTestCrash.h"

#include "ids.h"

#include "UtilLog.h"

namespace util {

//////////////////////////////////////////////////////////////////////////
// ASSSERT
//////////////////////////////////////////////////////////////////////////

void triggerAssertion()
{
    ASSERT_EQUALS(1,2);
}

//////////////////////////////////////////////////////////////////////////
// ACCESS VIOLATION
//////////////////////////////////////////////////////////////////////////

void triggerAccessViolation()
{

    struct Crasher { virtual void nonexist() = 0; };
    Crasher* crash = 0;
    crash->nonexist();
}

//////////////////////////////////////////////////////////////////////////
// DIVIDE BY ZERO
//////////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4723 ) // yes, 'potential' divide by zero...
void triggerDivideByZeroException()
{
    int j = 0;
    VAR_ERROR(1/j);
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS ASSERTION
//////////////////////////////////////////////////////////////////////////

void triggerWxAssertion()
{
    wxArrayString arr;
    arr[0];
 }

//////////////////////////////////////////////////////////////////////////
// UNHANDLED UNKNOWN EXCEPTION
//////////////////////////////////////////////////////////////////////////

void triggerUnknownException()
{
    throw 4;
}

//////////////////////////////////////////////////////////////////////////
// UNHANDLED BOOST EXCEPTION
//////////////////////////////////////////////////////////////////////////

void triggerBoostException()
{
    std::string s = "a125";
    std::istringstream is (s,std::istringstream::in);
    boost::archive::xml_iarchive ar(is);
    int i;
    ar & BOOST_SERIALIZATION_NVP(i);
}

//////////////////////////////////////////////////////////////////////////
// UNHANDLED STD EXCEPTION
//////////////////////////////////////////////////////////////////////////

void triggerStdException()
{
    std::string ().replace (100, 1, 1, 'c');
}

//////////////////////////////////////////////////////////////////////////
// INVALID PARAMETER
//////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
// Apparently GCC detects this bug with a warning ;-)
// BUt here, it's the bug that must be triggered...
void triggerInvalidParameter()
{
   char *formatString = 0;
   printf(formatString);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


//////////////////////////////////////////////////////////////////////////
// PURE VIRTUAL CALL
//////////////////////////////////////////////////////////////////////////

struct Derived;
struct Base
{
   Base(Derived *derived): mDerived(derived) {};
   ~Base();
   virtual void function(void) = 0;
   Derived* mDerived;
};

struct Derived : public Base
{
   Derived() : Base(this) {};   // C4355
   virtual void function(void) {};
};

Base::~Base()
{
     mDerived->function();
}

void triggerPureVirtualCall()
{
    Derived d;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

const int FirstId = meID_LAST + 1;

typedef boost::function<void()> Method;
typedef std::pair<wxString,util::Method> CrashInfo;
const std::vector< CrashInfo > crashes = boost::assign::list_of
    (std::make_pair("Access Violation", boost::bind(&triggerAccessViolation)))
    (std::make_pair("Assert", boost::bind(&triggerAssertion)))
    (std::make_pair("Divide by zero exception", boost::bind(&triggerDivideByZeroException)))
#ifdef _MSC_VER
    (std::make_pair("Invalid parameter", boost::bind(&triggerInvalidParameter)))
#endif
    (std::make_pair("Pure virtual call", boost::bind(&triggerPureVirtualCall)))
    (std::make_pair("Unhandled boost exception", boost::bind(&triggerBoostException)))
    (std::make_pair("Unhandled std exception", boost::bind(&triggerStdException)))
    (std::make_pair("Unhandled unknown exception", boost::bind(&triggerUnknownException)))
    (std::make_pair("WX assert", boost::bind(&triggerWxAssertion)))
;

void TestCrash::onCrashTest(wxCommandEvent& event)
{
    int index = event.GetId() - FirstId;
    bool inThread = (index % 2 == 1);
    index = index / 2;

    wxString description = crashes[index].first;
    util::Method method = crashes[index].second;

    VAR_ERROR(description)(inThread);
    if (inThread)
    {
        new boost::thread(boost::bind(method));
    }
    else
    {
        method();
    }
}

TestCrash::TestCrash(wxWindow* window)
    :   mWindow(window)
    ,   mMenu(new wxMenu())
{
    int id = FirstId;
    for ( CrashInfo crash : crashes )
    {
        mMenu->Append(id, crash.first);
        mWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &TestCrash::onCrashTest, this, id);
        id++;
        mMenu->Append(id, crash.first + " in thread");
        mWindow->Bind(wxEVT_COMMAND_MENU_SELECTED, &TestCrash::onCrashTest, this, id);
        id++;
    }
}

TestCrash::~TestCrash()
{
    for (unsigned int i = FirstId; i < FirstId + (2 * crashes.size()); ++i)
    {
        mWindow->Unbind(wxEVT_COMMAND_MENU_SELECTED,  &TestCrash::onCrashTest, this, i);
    }
}

wxMenu* TestCrash::getMenu() const
{
    return mMenu;
}

} // namespace
