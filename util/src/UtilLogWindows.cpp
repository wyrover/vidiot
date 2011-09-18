#include "UtilLogWindows.h"

std::ostream& operator<< (std::ostream& os, const EXCEPTION_POINTERS& obj)
{
#define LOG_EXCEPTION(code) case code: os << #code;  break;
    switch (obj.ExceptionRecord->ExceptionCode)
    {
        LOG_EXCEPTION(EXCEPTION_ACCESS_VIOLATION);
        LOG_EXCEPTION(EXCEPTION_DATATYPE_MISALIGNMENT);
        LOG_EXCEPTION(EXCEPTION_BREAKPOINT);
        LOG_EXCEPTION(EXCEPTION_SINGLE_STEP);
        LOG_EXCEPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
        LOG_EXCEPTION(EXCEPTION_FLT_DENORMAL_OPERAND);
        LOG_EXCEPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO);
        LOG_EXCEPTION(EXCEPTION_FLT_INEXACT_RESULT);
        LOG_EXCEPTION(EXCEPTION_FLT_INVALID_OPERATION);
        LOG_EXCEPTION(EXCEPTION_FLT_OVERFLOW);
        LOG_EXCEPTION(EXCEPTION_FLT_STACK_CHECK);
        LOG_EXCEPTION(EXCEPTION_FLT_UNDERFLOW);
        LOG_EXCEPTION(EXCEPTION_INT_DIVIDE_BY_ZERO);
        LOG_EXCEPTION(EXCEPTION_INT_OVERFLOW);
        LOG_EXCEPTION(EXCEPTION_PRIV_INSTRUCTION);
        LOG_EXCEPTION(EXCEPTION_IN_PAGE_ERROR);
        LOG_EXCEPTION(EXCEPTION_ILLEGAL_INSTRUCTION);
        LOG_EXCEPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION);
        LOG_EXCEPTION(EXCEPTION_STACK_OVERFLOW);
        LOG_EXCEPTION(EXCEPTION_INVALID_DISPOSITION);
        LOG_EXCEPTION(EXCEPTION_GUARD_PAGE);
        LOG_EXCEPTION(EXCEPTION_INVALID_HANDLE);
    default:
        os << "???";
    }

    PEXCEPTION_RECORD per = obj.ExceptionRecord;
    os  << '|' << std::hex 
        << per->ExceptionAddress    << '|'
        << per->ExceptionFlags      << '|'
        << per->ExceptionRecord     << '|'
        << per->NumberParameters;

    if (per->NumberParameters > 0)
    {
        os << '|';
        switch (per->ExceptionInformation[0])
        {
        case 0: os << "Read error"; break;
        case 1: os << "Write error"; break;
        case 8: os << "DEP"; break;
        default: os << "Unknown error";
        }
        if (per->NumberParameters > 1)
        {
            os  << " at location 0x" << std::hex << per->ExceptionInformation[1];
            if (per->NumberParameters > 2)
            {
                os  << " with status code" << std::hex << per->ExceptionInformation[2];
            }
        }
    }
    return os;
}

