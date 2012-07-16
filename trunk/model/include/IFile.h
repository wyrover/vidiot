#ifndef MODEL_I_FILE_H
#define MODEL_I_FILE_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include "IControl.h"
#include "UtilCloneable.h"

namespace model {

class IFile;
typedef boost::shared_ptr<IFile> IFilePtr;

class IFile
    :   public IControl
    ,   public ICloneable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    IFile();

    virtual ~IFile() {};

    //////////////////////////////////////////////////////////////////////////
    // ICLONEABLE
    //////////////////////////////////////////////////////////////////////////

    virtual IFile* clone() const override = 0;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    IFile(const IFile& other);

private:

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
BOOST_CLASS_VERSION(model::IFile, 1)

#endif // MODEL_I_FILE_H