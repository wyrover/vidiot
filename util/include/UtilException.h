// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

template <typename F>
void CatchExceptions(F f, std::function<void()> onException = nullptr)
{
    try
    {
        f();
    }
    catch (boost::exception &e)                  
    { 
        if (onException) 
        { 
            onException(); 
        }
        FATAL(boost::diagnostic_information(e)); 
    }
    catch (std::exception& e)                    
    { 
        if (onException)
        {
            onException();
        }
        FATAL(e.what());
    }
    catch (...)
    {
        LOG_ERROR;
        if (onException)
        {
            onException();
        }
        throw; // rethrow to allow the main exception handler to catch this and provide extra info (example: divide by zero error)
    }
}