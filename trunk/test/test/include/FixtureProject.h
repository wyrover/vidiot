#ifndef FIXTURE_PROJECT_H
#define FIXTURE_PROJECT_H

#include <list>
#include <wx/filename.h>
#include <boost/shared_ptr.hpp>

namespace model {
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace test {

class FixtureProject 
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FixtureProject();
    virtual ~FixtureProject();

    /// Create objects here. During construction the application fixture is
    /// not yet fully created. This method must be called in a test's setup
    /// method.
    void init();

    /// Destruct objects here. This method must be called in a test's 
    /// teardown method.
    void destroy();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxFileName TestFilesPath;               ///< Contains the path to the test files
    model::IPaths InputFiles;               ///< Contains the filenames of the input files in the test directory

    model::FolderPtr mRoot;
    model::FolderPtr mAutoFolder;
    model::SequencePtr mSequence;
};

} // namespace

#endif // FIXTURE_PROJECT_H