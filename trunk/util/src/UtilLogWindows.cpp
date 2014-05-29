// Copyright 2013,2014 Eric Raijmakers.
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

#include "UtilLogWindows.h"

#ifdef _MSC_VER

std::ostream& operator<<(std::ostream& os, const EXCEPTION_POINTERS& obj)
{
#define LOG_EXCEPTION(code) case code: os << #code;  break
    switch (obj.ExceptionRecord->ExceptionCode)
    {
        LOG_EXCEPTION(EXCEPTION_ACCESS_VIOLATION);
        LOG_EXCEPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
        LOG_EXCEPTION(EXCEPTION_BREAKPOINT);
        LOG_EXCEPTION(EXCEPTION_DATATYPE_MISALIGNMENT);
        LOG_EXCEPTION(EXCEPTION_FLT_DENORMAL_OPERAND);
        LOG_EXCEPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO);
        LOG_EXCEPTION(EXCEPTION_FLT_INEXACT_RESULT);
        LOG_EXCEPTION(EXCEPTION_FLT_INVALID_OPERATION);
        LOG_EXCEPTION(EXCEPTION_FLT_OVERFLOW);
        LOG_EXCEPTION(EXCEPTION_FLT_STACK_CHECK);
        LOG_EXCEPTION(EXCEPTION_FLT_UNDERFLOW);
        LOG_EXCEPTION(EXCEPTION_GUARD_PAGE);
        LOG_EXCEPTION(EXCEPTION_ILLEGAL_INSTRUCTION);
        LOG_EXCEPTION(EXCEPTION_IN_PAGE_ERROR);
        LOG_EXCEPTION(EXCEPTION_INT_DIVIDE_BY_ZERO);
        LOG_EXCEPTION(EXCEPTION_INT_OVERFLOW);
        LOG_EXCEPTION(EXCEPTION_INVALID_DISPOSITION);
        LOG_EXCEPTION(EXCEPTION_INVALID_HANDLE);
        LOG_EXCEPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION);
        LOG_EXCEPTION(EXCEPTION_PRIV_INSTRUCTION);
        LOG_EXCEPTION(EXCEPTION_SINGLE_STEP);
        LOG_EXCEPTION(EXCEPTION_STACK_OVERFLOW);
    default:
        os << std::hex << obj.ExceptionRecord->ExceptionCode;
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

#endif
