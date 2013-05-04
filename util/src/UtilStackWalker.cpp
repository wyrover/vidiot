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