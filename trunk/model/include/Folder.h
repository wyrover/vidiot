#ifndef FOLDER_H
#define FOLDER_H

#include <wx/string.h>
#include "Node.h"

namespace model {

class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;

class Folder
    :   public Node
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Recovery constructor
    Folder();

    Folder(wxString name);

    virtual ~Folder();

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

#endif // FOLDER_H