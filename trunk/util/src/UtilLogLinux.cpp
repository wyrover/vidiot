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
    #define LOG_SIGNAL(enumvalue) case enumvalue: os << #enumvalue << '(' << enumvalue << ')';  break
    switch (signal->si_signo)
    {
        LOG_SIGNAL(SIGHUP);
        LOG_SIGNAL(SIGINT);
        LOG_SIGNAL(SIGQUIT);
        LOG_SIGNAL(SIGILL);
        LOG_SIGNAL(SIGTRAP);
        LOG_SIGNAL(SIGABRT);
        LOG_SIGNAL(SIGBUS);
        LOG_SIGNAL(SIGFPE);
        LOG_SIGNAL(SIGSEGV);
        LOG_SIGNAL(SIGPIPE);
        LOG_SIGNAL(SIGTERM);
        LOG_SIGNAL(SIGALRM);
        LOG_SIGNAL(SIGSTKFLT);
        LOG_SIGNAL(SIGCHLD);
        LOG_SIGNAL(SIGTSTP);
        LOG_SIGNAL(SIGSYS);
        default: os << "Unknown" << '(' << signal->si_signo << ')'; break;
    }

    os << " code " << signal->si_code;
    os << " errno " << signal->si_errno;
    return os;
}

#endif
