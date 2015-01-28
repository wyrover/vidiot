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

#ifndef ROOT_H
#define ROOT_H

#include "Folder.h"

namespace model {

/// Note: During copy/paste activites additional Root nodes may be constructed.
///       Hence, not a 'single instance'.
class Root
    : public Folder
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Root();
    virtual ~Root();
    wxString getName() const override;

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    static bool exists();
    static std::vector<Root*> sInstance;

private:

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::Root, 1)
BOOST_CLASS_EXPORT_KEY(model::Root)

#endif
