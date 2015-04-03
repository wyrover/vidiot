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

#include "UtilLogX.h"

#ifdef __GNUC__

std::ostream& operator<<(std::ostream& os, XErrorEvent* e)
{
    os << "serial " << e->serial;
    os << " error_code ";
    #define LOG_TYPE(errortype) case errortype: os << #errortype << '(' << errortype << ')';  break
    switch (e->error_code)
    {
        LOG_TYPE(Success);
        LOG_TYPE(BadRequest);           // bad request code
        LOG_TYPE(BadValue);             // int parameter out of range
        LOG_TYPE(BadWindow);            // parameter not a Window
        LOG_TYPE(BadPixmap);            // parameter not a Pixmap
        LOG_TYPE(BadAtom);              // parameter not an Atom
        LOG_TYPE(BadCursor);            // parameter not a Cursor
        LOG_TYPE(BadFont);              // parameter not a Font
        LOG_TYPE(BadMatch);             // parameter mismatch
        LOG_TYPE(BadDrawable);          // parameter not a Pixmap or Window
        LOG_TYPE(BadAccess);            // depending on context:
                                        // key/button already grabbed
                                        // attempt to free an illegal cmap entry
                                        // attempt to store into a read-only color map entry.
                                        // attempt to modify the access control list from other than the local host.
        LOG_TYPE(BadAlloc);             // insufficient resources
        LOG_TYPE(BadColor);             // no such colormap
        LOG_TYPE(BadGC);                // parameter not a GC
        LOG_TYPE(BadIDChoice);          // choice not in range or already used
        LOG_TYPE(BadName);              // font or color name doesn't exist
        LOG_TYPE(BadLength);            // Request length incorrect
        LOG_TYPE(BadImplementation);    // server is defective
        default: os << "Unknown" << '(' << e->error_code << ')'; break;
    }
    os << " request_code " << static_cast<unsigned int>(e->request_code);
    os << " minor_code " << static_cast<unsigned int>(e->minor_code);
    os << " type " << static_cast<unsigned int>(e->type);
    return os;
}

#endif
