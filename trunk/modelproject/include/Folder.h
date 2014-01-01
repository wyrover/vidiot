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

#ifndef FOLDER_H
#define FOLDER_H

#include "Node.h"

namespace model {

class Folder
    :   public Node
{
public:

    //////////////////////////////////////////////////////////////////////////
    // ROOT
    //////////////////////////////////////////////////////////////////////////

    static FolderPtr makeRoot();

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Recovery constructor
    Folder();

    Folder(wxString name);

    virtual ~Folder();

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    void check() override; ///< Update the autofolder children. The folder is synced with the filesystem.

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getName() const override;
    void setName(wxString name) override;

    /// Return a name for use when the folder is used to create a sequence.
    virtual wxString getSequenceName() const;

private:

    wxString mName;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Folder, 1)
BOOST_CLASS_EXPORT_KEY(model::Folder)

#endif // FOLDER_H