#ifndef MODEL_I_PATH_H
#define MODEL_I_PATH_H

#include <wx/filename.h>
#include <list>
#include <boost/shared_ptr.hpp>

namespace model {

class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;

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
};

} // namespace

#endif // MODEL_I_PATH_H
