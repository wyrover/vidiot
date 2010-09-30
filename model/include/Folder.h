#ifndef FOLDER_H
#define FOLDER_H

#include <wx/string.h>
#include "AProjectViewNode.h"

namespace model {

class Folder
    :   public AProjectViewNode
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /** Recovery constructor */
    Folder();

    Folder(wxString name);

    ~Folder();

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getName() const;
    void setName(wxString name);

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

BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::Folder);
// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Folder, 1)
BOOST_CLASS_EXPORT(model::Folder)

#endif // FOLDER_H
