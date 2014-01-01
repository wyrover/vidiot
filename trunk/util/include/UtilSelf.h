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

#ifndef UTIL_SELF_H
#define UTIL_SELF_H

#include "UtilLog.h"

/// Everywhere where shared_from_this is used,
/// make sure that exceptions are caught for
/// having the best stack trace.
template <typename CLASS>
class Self
    :   public boost::enable_shared_from_this<CLASS>
{
public:

    boost::shared_ptr<CLASS> self()
    {
        boost::shared_ptr<CLASS> result;
        try
        {
            result = shared_from_this();
        }
        catch (boost::exception &e)
        {
            FATAL(boost::diagnostic_information(e));
        }
        return result;
    }
};

#endif // UTIL_SELF_H