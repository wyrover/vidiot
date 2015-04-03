// Copyright 2015 Eric Raijmakers.
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

#include "UtilLogLinux.h"

#ifdef __GNUC__

std::ostream& operator<<(std::ostream& os, siginfo_t* signal)
{
    #define LOG_ENUM(enumvalue) case enumvalue: os << #enumvalue << '(' << enumvalue << ')';  break
    switch (signal->si_signo)
    {
        LOG_ENUM(SIGHUP);
        LOG_ENUM(SIGINT);
        LOG_ENUM(SIGQUIT);
        LOG_ENUM(SIGILL);
        LOG_ENUM(SIGTRAP);
        LOG_ENUM(SIGABRT);
        LOG_ENUM(SIGBUS);
        LOG_ENUM(SIGFPE);
        LOG_ENUM(SIGSEGV);
        LOG_ENUM(SIGPIPE);
        LOG_ENUM(SIGTERM);
        LOG_ENUM(SIGALRM);
        LOG_ENUM(SIGSTKFLT);
        LOG_ENUM(SIGCHLD);
        LOG_ENUM(SIGTSTP);
        LOG_ENUM(SIGSYS);
        default: os << "Unknown" << '(' << signal->si_signo << ')'; break;
    }

    os << " code " << signal->si_code;
    os << " errno " << signal->si_errno;
    return os;
}

#endif
