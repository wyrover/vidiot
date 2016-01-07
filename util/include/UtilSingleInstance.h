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

/// This template gives a class a CLASS::get() method by which the one and only instantiated object of the class can be obtained.
/// This only works if there's only one instance of the class.
template <class INSTANCETYPE>
class SingleInstance
{
public:
    SingleInstance()
    {
        ASSERT_ZERO(sInstance);
        sInstance = this;
    }
    virtual ~SingleInstance()
    {
        ASSERT_NONZERO(sInstance);
        sInstance = 0;
    }
    static INSTANCETYPE& get()
    {
        ASSERT_NONZERO(sInstance);
        return *(static_cast<INSTANCETYPE*>(sInstance));
    }
    static bool exists()
    {
        return sInstance != 0;
    }
private:
    static SingleInstance* sInstance;
};

template <class INSTANCETYPE> SingleInstance<INSTANCETYPE>* SingleInstance<INSTANCETYPE>::sInstance = nullptr;
