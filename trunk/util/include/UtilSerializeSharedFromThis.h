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

#ifndef UTIL_SERIALIZE_SHARED_FROM_THIS_H
#define UTIL_SERIALIZE_SHARED_FROM_THIS_H

// Trick from: http://blog.asdfa.net/using-shared_from_this-inside-boostserialization/
// This ensures that a shared ptr to an object is always present before
// the actual serialization of the object is done. That avoids 'bad_weak_ptr'
// exceptions during deserialization.

#define ENABLE_SHARED_FROM_THIS_DURING_DESERIALIZATION(classname)   \
                                                                    \
namespace boost { namespace serialization {                         \
                                                                    \
inline void load_construct_data(boost::archive::xml_iarchive &ar,   \
                                classname *obj,                     \
                                const unsigned int archiveVersion)  \
{                                                                   \
    boost::shared_ptr<classname> sharedPtr;                         \
    ::new(obj)classname();                                          \
    ar.reset(sharedPtr, obj);                                       \
};                                                                  \
                                                                    \
}}                                                                  \

#endif //UTIL_SERIALIZE_SHARED_FROM_THIS_H