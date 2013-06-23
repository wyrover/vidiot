#ifndef FOLDER_H
#define FOLDER_H

#include "Node.h"

namespace model {

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

#endif // FOLDER_H