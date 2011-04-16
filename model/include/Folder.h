#ifndef FOLDER_H
#define FOLDER_H

#include <wx/string.h>
#include "AProjectViewNode.h"

namespace model {

class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;

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

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Folder, 1)

#endif // FOLDER_H
