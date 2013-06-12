#ifndef FIXTURE_PROJECT_H
#define FIXTURE_PROJECT_H

#include <list>
#include <vector>
#include <wx/filename.h>
#include <boost/shared_ptr.hpp>
#include "UtilInt.h"

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

    FixtureProject(bool focusTimeline = true);
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

    bool mFocusTimeline;                    ///< True if the timeline must be focused
    model::IPaths InputFiles;               ///< Contains the filenames of the input files in the test directory

    model::FolderPtr mRoot;
    model::FolderPtr mAutoFolder;
    model::SequencePtr mSequence;

    pts OriginalLengthOfVideoClip(int trackindex = 0, int clipindex = 0);
    pts OriginalPtsOfVideoClip(int trackindex = 0, int clipindex = 0);
    pts OriginalLengthOfAudioClip(int trackindex = 0, int clipindex = 0);
    pts OriginalPtsOfAudioClip(int trackindex = 0, int clipindex = 0);

    std::vector<pts> mOriginalLengthOfVideoClip;
    std::vector<pts> mOriginalPtsOfVideoClip;
    std::vector<pts> mOriginalLengthOfAudioClip;
    std::vector<pts> mOriginalPtsOfAudioClip;
};

} // namespace

#endif // FIXTURE_PROJECT_H