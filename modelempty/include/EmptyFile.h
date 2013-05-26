#ifndef MODEL_EMPTY_FILE_H
#define MODEL_EMPTY_FILE_H

#include "IFile.h"

namespace model {

class EmptyFile
    :   public IFile
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyFile();
    virtual EmptyFile* clone() const override;
    virtual ~EmptyFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    EmptyFile(const EmptyFile& other);

};

} // namespace

#endif // MODEL_EMPTY_FILE_H