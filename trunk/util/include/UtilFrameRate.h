// Copyright 2013-2015 Eric Raijmakers.
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

#include "libavutil/rational.h"
#include "UtilInt.h"

class FrameRate
    : public rational64
{
public:

    static const FrameRate s24p;
    static const FrameRate s25p;
    static const FrameRate s30p;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit FrameRate(const int64_t& num, const int64_t& den);
    explicit FrameRate(const AVRational& avr);
    explicit FrameRate(const wxString& framerate);
    explicit FrameRate(const rational64& r);

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    static std::vector<FrameRate> getSupported();

    wxString toString() const;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const FrameRate& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

inline bool operator<(FrameRate l, const FrameRate& r) 
{ 
    return 
        l.numerator() < r.numerator() || 
        (l.numerator() == r.numerator() && l.denominator() < r.denominator()); 
}


//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(FrameRate, 1)
