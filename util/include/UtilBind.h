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

#pragma once

/// Using this macro (instead of generator->Bind(...) directly) ensures
/// that a bit of extra information is logged whenever an unhandled exception
/// occurs in boost/std: the name of the event handler method that triggered
/// the exception.
///
/// Note: only for GUI events this is required. Model events are triggered

#define BindAndCatchExceptions(generator, eventType, method, handler, ...)\
    ExceptionSafeBind(generator, eventType, method, handler, #method, ## __VA_ARGS__ )

template <typename EventGenerator, typename EventHandler, typename EventTag, typename EventArg>
void ExceptionSafeBind(
    EventGenerator* generator,
    const EventTag &eventType,
    void (EventHandler::*method)(EventArg &),
    EventHandler *handler,
    std::string methodName,
    int winid = wxID_ANY)
{
    generator->Bind(eventType, [methodName, handler, method](EventArg& event)
    {
        try
        {
            (handler->*method)(event);
        }
        catch (boost::exception &e)
        {
            FATAL(methodName)(boost::diagnostic_information(e));
        }
        catch (std::exception& e)
        {
            FATAL(methodName)(e.what());
        }
        catch (...)
        {
            FATAL(methodName);
        }
    }, winid);
}

template <typename EventGenerator, typename EventHandler, typename EventTag, typename EventArg>
void ExceptionSafeBind(
    EventGenerator& generator,
    const EventTag &eventType,
    void (EventHandler::*method)(EventArg &),
    EventHandler *handler,
    std::string methodName,
    int winid = wxID_ANY)
{
    generator.Bind(eventType, [methodName, handler, method](EventArg& event)
    {
        try
        {
            (handler->*method)(event);
        }
        catch (boost::exception &e)
        {
            FATAL(methodName)(boost::diagnostic_information(e));
        }
        catch (std::exception& e)
        {
            FATAL(methodName)(e.what());
        }
        catch (...)
        {
            FATAL(methodName);
        }
    }, winid);
}
