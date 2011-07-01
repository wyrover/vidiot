#include "ProjectViewCreateSequence.h"
#include "UtilLog.h"
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "AutoFolder.h"
#include "Window.h"
#include "TimeLinesView.h"
#include "Folder.h"
#include "Sequence.h"
#include "TrackCreator.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewCreateSequence::ProjectViewCreateSequence(model::FolderPtr folder)
    :   ProjectViewCommand()
    ,   mName(folder->getName())
    ,   mParent(findFirstNonAutoFolderParent(folder))
    ,   mInputFolder(folder)
    ,   mSequence()
{
    VAR_INFO(this)(mParent)(mInputFolder);
    ASSERT(mParent); // Parent folder must exist
    mCommandName = _("Create sequence from folder ") + mName;
}

ProjectViewCreateSequence::ProjectViewCreateSequence(model::FolderPtr folder, wxString name)
    :   ProjectViewCommand()
    ,   mName(name)
    ,   mParent(folder)
    ,   mInputFolder()
    ,   mSequence()
{
    VAR_INFO(mParent)(name);
    ASSERT(mParent); // Parent folder must exist
    ASSERT(!folder->isA<model::AutoFolder>());
    mCommandName = _("Create sequence ") + mName;
}

ProjectViewCreateSequence::~ProjectViewCreateSequence()
{
}

//////////////////////////////////////////////////////////////////////////
// WXCOMMAND
//////////////////////////////////////////////////////////////////////////

bool ProjectViewCreateSequence::Do()
{
    VAR_INFO(this);
    if (!mSequence)
    {
        mSequence = boost::make_shared<model::Sequence>(mName);

        if (mInputFolder)
        {
            TrackCreator c(mInputFolder->getChildren());
            mSequence->addVideoTracks(boost::assign::list_of(c.getVideoTrack()));
            mSequence->addAudioTracks(boost::assign::list_of(c.getAudioTrack()));
        }
    }

    mParent->addChild(mSequence);
    gui::Window::get().getTimeLines().Open(mSequence);

    // May never return false, since then the command is deleted by the command processor. See the use of this object in projectview.
    return true;
}

bool ProjectViewCreateSequence::Undo()
{
    VAR_INFO(this);
    mParent->removeChild(mSequence);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::SequencePtr ProjectViewCreateSequence::getSequence() const
{
    return mSequence;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::FolderPtr ProjectViewCreateSequence::findFirstNonAutoFolderParent(model::ProjectViewPtr node) const
{
    model::ProjectViewPtr parent = node->getParent();
    ASSERT(parent)(node);

    while (parent->isA<model::AutoFolder>())
    {
        parent = parent->getParent(); 

    }
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(parent);
    ASSERT(folder)(parent);
    return folder;
}

} // namespace