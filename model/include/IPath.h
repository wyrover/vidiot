#ifndef MODEL_I_PATH_H
#define MODEL_I_PATH_H

namespace model {

/// Interface class for objects that are files/directories on disk
class IPath
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual ~IPath() {};

    //////////////////////////////////////////////////////////////////////////
    // PATH
    //////////////////////////////////////////////////////////////////////////

    /// \return absolute path to the object on disk.
    virtual wxFileName getPath() const = 0;

    /// Is called when the object should do a check for consistency.
    /// i.e. Is the file/folder on disk still present?
    virtual void check() = 0;
};

} // namespace

#endif // MODEL_I_PATH_H