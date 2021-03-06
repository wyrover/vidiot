// Copyright 2014-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "FileAnalyzer.h"

#include "AudioFile.h"
#include "Config.h"
#include "INode.h"
#include "Project.h"
#include "ProjectModification.h"
#include "ProjectViewAddAsset.h"
#include "ProjectView.h"
#include "StatusBar.h"
#include "UtilWindow.h"
#include "VideoFile.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FileAnalyzer::FileAnalyzer(wxStrings fileNames, wxWindow* parent)
    : mParent(parent)
    , mMostFrequentVideoSize(Config::get().read<int>(Config::sPathVideoDefaultWidth), Config::get().read<int>(Config::sPathVideoDefaultHeight))
	, mMostFrequentFrameRate(Config::get().read<wxString>(Config::sPathVideoDefaultFrameRate))
    , mMostFrequentAudioRate(std::make_pair(Config::get().read<int>(Config::sPathAudioDefaultSampleRate), Config::get().read<int>(Config::sPathAudioDefaultNumberOfChannels)))
    , mVideoSizeOccurrence({ { mMostFrequentVideoSize, 0 } })
    , mFrameRateOccurrence({ { mMostFrequentFrameRate, 0 } })
    , mAudioRateOccurrence({ { mMostFrequentAudioRate, 0 } })
{
	for (wxString path : fileNames)
	{
		mFileNames.emplace_back(::util::path::toFileName(path));
	}
	init();
}

FileAnalyzer::FileAnalyzer(const wxArrayString& fileNames, wxWindow* parent)
	: mParent(parent)
	, mMostFrequentVideoSize(Config::get().read<int>(Config::sPathVideoDefaultWidth), Config::get().read<int>(Config::sPathVideoDefaultHeight))
	, mMostFrequentFrameRate(Config::get().read<wxString>(Config::sPathVideoDefaultFrameRate))
	, mMostFrequentAudioRate(std::make_pair(Config::get().read<int>(Config::sPathAudioDefaultSampleRate), Config::get().read<int>(Config::sPathAudioDefaultNumberOfChannels)))
    , mVideoSizeOccurrence({ { mMostFrequentVideoSize, 0 } })
    , mFrameRateOccurrence({ { mMostFrequentFrameRate, 0 } })
    , mAudioRateOccurrence({ { mMostFrequentAudioRate, 0 } })
{
	for (wxString filename : fileNames)
	{
		mFileNames.emplace_back(::util::path::toFileName(filename));
	}
	init();
}

void FileAnalyzer::init()
{
	if (mParent != nullptr)
	{
		mDialog = boost::make_shared<wxProgressDialog>(_("Indexing files"), wxEmptyString, 100, mParent);
		mDialog->SetWindowStyleFlag(wxPD_APP_MODAL | wxPD_ELAPSED_TIME); //| wxPD_AUTO_HIDE
		updateProgressDialog();
	}

    std::sort(mFileNames.begin(), mFileNames.end(), [](const wxFileName& file1, const wxFileName& file2) { return file1.GetLongPath() < file2.GetLongPath(); });

	// Check if files can be opened, and determine the most frequent frame/sample rates of the files.

	for (const wxFileName& filename : mFileNames)
	{
		if (filename.Exists())
		{
			if (filename.IsDir())
			{
				mNodes.emplace_back(boost::make_shared<AutoFolder>(filename));
				indexFolder(filename, mFileNames.size() != 1); // Do not recurse if only one folder is given
			}
            else if (wxFileName(filename).GetExt().IsSameAs(Project::sFileExtension))
            {
                mNumberOfProjects++;
            }
            else
			{
				FilePtr file = indexFile(filename);
				if (file)
				{
					mNodes.emplace_back(file);
				}
			}
		}
	}

	if (mMostFrequentFrameRate == FrameRate(250000, 10427))
	{
		// Some codecs use this framerate iso s24p.
		mMostFrequentFrameRate = FrameRate::s24p;
	}

	mDialog.reset();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool FileAnalyzer::isProjectOnly() const
{
    return mNumberOfProjects == 1;
}

wxString FileAnalyzer::getProject() const
{
    ASSERT(isProjectOnly());
    return util::path::toPath(mFileNames.front());
}

int FileAnalyzer::getNumberOfFolders() const
{
    return std::count_if(mFileNames.begin(),mFileNames.end(), [](wxFileName file) -> bool { return file.IsDir(); });
}

int FileAnalyzer::getNumberOfFiles() const
{
    return std::count_if(mFileNames.begin(),mFileNames.end(), [](wxFileName file) -> bool { return !file.IsDir(); });
}

int FileAnalyzer::getNumberOfMediaFiles() const
{
    return mNumberOfMediaFiles;
}

NodePtrs FileAnalyzer::getNodes() const
{
    return mNodes;
}

wxString FileAnalyzer::getFirstFolderName() const
{
    for (model::NodePtr node : mNodes)
    {
        if (node->isA<model::AutoFolder>())
        {
            return node->getName();
        }
    }
    FATAL("No folder in list");
    return "";
}


wxSize FileAnalyzer::getMostFrequentVideoSize() const
{
    return mMostFrequentVideoSize;
}

FrameRate FileAnalyzer::getMostFrequentFrameRate() const
{
    return mMostFrequentFrameRate;
}

std::pair<int,int> FileAnalyzer::getMostFrequentAudioRate() const
{
    return mMostFrequentAudioRate;
}

void FileAnalyzer::addNodesToProjectView() const
{
    for (model::NodePtr node : mNodes)
    {
        if (::gui::ProjectView::get().findConflictingName(::model::Project::get().getRoot(), node->getName(), node->isA<model::Folder>() ? ::gui::NODETYPE_FOLDER : ::gui::NODETYPE_FILE))
        {
            return;
        }
    }
    if (model::ProjectModification::submitIfPossible(new cmd::ProjectViewAddAsset(model::Project::get().getRoot(), mNodes)))
    {
        for (model::NodePtr node : mNodes)
        {
            node->check(true); // Update any added autofolders
        }
    }
}

bool FileAnalyzer::checkIfOkForPasteOrDrop() const
{
	for (model::NodePtr node : mNodes)
	{
		if (!node->isA<model::File>())
		{
			gui::StatusBar::get().timedInfoText(_("Only regular files can be pasted/dropped."));
			return false;
		}
	}
	if (mNodes.empty())
	{
		gui::StatusBar::get().timedInfoText(_("Some of these files cannot be used."));
		return false;
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void FileAnalyzer::indexFolder(const wxFileName& dirName, bool recurse)
{
    ASSERT(dirName.IsDir())(dirName);
    wxDir dir(dirName.GetLongPath() );
    if (!dir.IsOpened())
    {
        return;
    }
    mFolders++;
    updateProgressDialog();

    wxFileNames subdirs;
    wxFileNames files;
    wxString nodename;
    for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_FILES); cont; cont = dir.GetNext(&nodename))
    {
        files.emplace_back(wxFileName(dirName.GetLongPath(), nodename));
    }
    for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_DIRS); cont; cont = dir.GetNext(&nodename))
    {
        wxFileName filename(dirName.GetLongPath(), "");
        filename.AppendDir(nodename);
        subdirs.emplace_back(filename);
    }

    for (const wxFileName& file : files)
    {
        indexFile(file);
    }
    if (recurse)
    {
        for (const wxFileName& subdir : subdirs)
        {
            indexFolder(subdir, true);
        }
    }
}

FilePtr FileAnalyzer::indexFile(const wxFileName& fileName)
{
    FilePtr file = boost::make_shared<File>(fileName);

    if (file->canBeOpened())
    {
        mNumberOfMediaFiles++;
        updateProgressDialog();
        if (file->getType() == FileType_Title)
        {
            // No frame/sample rate analysis required.
        }
        else
        {
            if (file->hasVideo())
            {
                VideoFilePtr videofile = boost::make_shared<VideoFile>(fileName);
                mFrameRateOccurrence[videofile->getFrameRate()] = mFrameRateOccurrence[videofile->getFrameRate()] + 1;
                if (mFrameRateOccurrence[videofile->getFrameRate()] > mFrameRateOccurrence[mMostFrequentFrameRate])
                {
                    mMostFrequentFrameRate = videofile->getFrameRate();
                }
                wxSize size = videofile->getSize();
                mVideoSizeOccurrence[size] = mVideoSizeOccurrence[size] + 1;
                if (mVideoSizeOccurrence[size] > mVideoSizeOccurrence[mMostFrequentVideoSize])
                {
                    mMostFrequentVideoSize = size;
                }
            }
            if (file->hasAudio())
            {
                AudioFilePtr audiofile = boost::make_shared<AudioFile>(fileName);
                std::pair<int, int> fileAudioRate = std::make_pair(audiofile->getSampleRate(), audiofile->getChannels());
                mAudioRateOccurrence[fileAudioRate]++;
                if (mAudioRateOccurrence[fileAudioRate] > mAudioRateOccurrence[mMostFrequentAudioRate])
                {
                    mMostFrequentAudioRate = fileAudioRate;
                }
            }
        }
    }
    else
    {
        file.reset();
    }
    return file;
}

void FileAnalyzer::updateProgressDialog()
{
    if (mDialog)
    {
        wxString progress;
        progress << wxString::Format(_("Found %d file(s)"), mNumberOfMediaFiles);
        mDialog->Pulse(progress);
    }
}

} //namespace
