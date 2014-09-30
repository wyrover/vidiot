// Copyright 2014 Eric Raijmakers.
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
#include "VideoFile.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FileAnalyzer::FileAnalyzer(wxStrings fileNames, wxWindow* parent)
    : mMostFrequentFrameRate(Config::ReadString(Config::sPathDefaultFrameRate))
    , mMostFrequentAudioRate(std::make_pair(Config::ReadLong(Config::sPathDefaultAudioSampleRate), Config::ReadLong(Config::sPathDefaultAudioChannels)))
    , mMostFrequentVideoSize(Config::ReadLong(Config::sPathDefaultVideoWidth), Config::ReadLong(Config::sPathDefaultVideoHeight))
    , mVideoSizeOccurrence(boost::assign::map_list_of(mMostFrequentVideoSize,0))
    , mFrameRateOccurrence(boost::assign::map_list_of(mMostFrequentFrameRate,0))
    , mAudioRateOccurrence(boost::assign::map_list_of(mMostFrequentAudioRate,0))
    , mNumberOfMediaFiles(0)
    , mFolders(0)
{
    if (parent != nullptr)
    {
        mDialog = boost::make_shared<wxProgressDialog>(_("Indexing files"),wxEmptyString,100,parent);
        mDialog->SetWindowStyleFlag(wxPD_APP_MODAL | wxPD_ELAPSED_TIME); //| wxPD_AUTO_HIDE 
        updateProgressDialog();
    }

    for (wxString path : fileNames)
    { 
        mFileNames.push_back(::util::path::toFileName(path));
    }
    mFileNames.sort([](const wxFileName& file1, const wxFileName& file2) { return file1.GetFullPath() < file2.GetFullPath(); });

    // Check if files can be opened, and determine the most frequent frame/sample rates of the files.

    for (const wxFileName& filename : mFileNames)
    {
        if (filename.Exists())
        {
            if (filename.IsDir())
            {
                mNodes.push_back(boost::make_shared<AutoFolder>(filename));
                indexFolder(filename, mFileNames.size() != 1); // Do not recurse if only one folder is given
            }
            else
            {
                FilePtr file = indexFile(filename);
                if (file)
                {
                    mNodes.push_back(file);
                }
            }
        }
    }

    if (mMostFrequentFrameRate == FrameRate(250000,10427))
    {
        // Some codecs use this framerate iso s24p.
        mMostFrequentFrameRate = FrameRate::s24p;
    }

    mDialog.reset();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

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
    if (!mNodes.empty())
    {
        model::ProjectModification::submit(new command::ProjectViewAddAsset(model::Project::get().getRoot(), mNodes)); 
        for (model::NodePtr node : mNodes)
        {
            node->check(true); // Update any added autofolders
        }
    }
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
        files.push_back(wxFileName(dirName.GetLongPath(), nodename));
    }
    for (bool cont = dir.GetFirst(&nodename,wxEmptyString,wxDIR_DIRS); cont; cont = dir.GetNext(&nodename))
    {
        wxFileName filename(dirName.GetLongPath(), "");
        filename.AppendDir(nodename);
        subdirs.push_back(filename);
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
            std::pair<int,int> fileAudioRate = std::make_pair(audiofile->getSampleRate(), audiofile->getChannels());
            mAudioRateOccurrence[fileAudioRate]++;
            if (mAudioRateOccurrence[fileAudioRate] > mAudioRateOccurrence[mMostFrequentAudioRate])
            {
                mMostFrequentAudioRate = fileAudioRate;
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
        progress << _("Found ") << mNumberOfMediaFiles << " files";
        if (mFolders > 1)
        {
            progress << _(" in ") << mFolders << _(" folders");
        }
        progress << '.';
        mDialog->Pulse(progress);
    }
}

} //namespace
