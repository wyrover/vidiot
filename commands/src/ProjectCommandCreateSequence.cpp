#include "ProjectCommandCreateSequence.h"
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

ProjectCommandCreateSequence::ProjectCommandCreateSequence(model::FolderPtr folder)
:   ProjectCommand()
,   mName(folder->getName())
,   mParent(boost::dynamic_pointer_cast<model::Folder>(folder->getParent()))
,   mInputFolder(folder)
,   mSequence()
{
    VAR_INFO(this)(mParent)(mInputFolder);
    mCommandName = _("Create sequence from folder ") + mName; 
}

ProjectCommandCreateSequence::ProjectCommandCreateSequence(model::FolderPtr folder, wxString name)
:   ProjectCommand()
,   mName(name)
,   mParent(folder)
,   mInputFolder()
,   mSequence()
{
    VAR_INFO(mParent)(name);
    ASSERT(mParent); // Parent folder must exist
    mCommandName = _("Create sequence ") + mName; 
}

ProjectCommandCreateSequence::~ProjectCommandCreateSequence()
{
}

bool ProjectCommandCreateSequence::Do()
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
                    model::VideoFilePtr videoFile = boost::make_shared<model::VideoFile>(file->getPath());
                    model::AudioFilePtr audioFile = boost::make_shared<model::AudioFile>(file->getPath());
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

    mParent->addChild(mSequence);
    // May never return false, since then the command is deleted by the command processor. See the use of this object in projectview.
    return true;
}

bool ProjectCommandCreateSequence::Undo()
{
    VAR_INFO(this);
    mParent->removeChild(mSequence);
    return true;

}

model::SequencePtr ProjectCommandCreateSequence::getSequence()
{
    return mSequence;
}
