#include "ProjectViewCreateSequence.h"
#include "UtilLog.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "AutoFolder.h"
#include "VideoFile.h"
#include "AudioFile.h"
#include "VideoClip.h"
#include "AudioClip.h"
#include "VideoTrack.h"
#include "AudioTrack.h"

namespace command {

ProjectViewCreateSequence::ProjectViewCreateSequence(model::FolderPtr folder)
:   ProjectViewCommand()
,   mName(folder->getName())
,   mParent(boost::dynamic_pointer_cast<model::Folder>(folder->getParent()))
,   mInputFolder(folder)
,   mSequence()
{
    VAR_INFO(this)(mParent)(mInputFolder);
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
    mCommandName = _("Create sequence ") + mName;
}

ProjectViewCreateSequence::~ProjectViewCreateSequence()
{
}

bool ProjectViewCreateSequence::Do()
{
    VAR_INFO(this);
    if (!mSequence)
    {
        mSequence = boost::make_shared<model::Sequence>(mName);
        model::VideoTrackPtr videoTrack = boost::make_shared<model::VideoTrack>();
        model::AudioTrackPtr audioTrack = boost::make_shared<model::AudioTrack>();
        mSequence->addVideoTrack(videoTrack);
        mSequence->addAudioTrack(audioTrack);

        if (mInputFolder)
        {
            model::AutoFolderPtr autofolder = boost::dynamic_pointer_cast<model::AutoFolder>(mInputFolder);
            if (autofolder)
            {
                autofolder->update();
            }
            BOOST_FOREACH( model::ProjectViewPtr child, mInputFolder->getChildren())
            {
                model::FilePtr file = boost::dynamic_pointer_cast<model::File>(child);
                if (file)
                {
                    VAR_DEBUG(file);
                    model::VideoFilePtr videoFile = boost::make_shared<model::VideoFile>(file->getPath());
                    VAR_DEBUG(videoFile);
                    model::AudioFilePtr audioFile = boost::make_shared<model::AudioFile>(file->getPath());
                    VAR_DEBUG(audioFile);
                    model::VideoClipPtr videoClip = boost::make_shared<model::VideoClip>(videoFile);
                    model::AudioClipPtr audioClip = boost::make_shared<model::AudioClip>(audioFile);
                    videoClip->setLink(audioClip);
                    audioClip->setLink(videoClip);
                    videoTrack->addVideoClip(videoClip);
                    audioTrack->addAudioClip(audioClip);
                }
            }
        }
    }

    /** @todo do not add this sequence to an autofolder. Then, I cannot move it or rename it anymore */
    mParent->addChild(mSequence);
    // May never return false, since then the command is deleted by the command processor. See the use of this object in projectview.
    return true;
}

bool ProjectViewCreateSequence::Undo()
{
    VAR_INFO(this);
    mParent->removeChild(mSequence);
    return true;

}

model::SequencePtr ProjectViewCreateSequence::getSequence()
{
    return mSequence;
}

} // namespace