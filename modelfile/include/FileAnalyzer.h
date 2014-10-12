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

#ifndef MODEL_FILE_ANALYZER_H
#define MODEL_FILE_ANALYZER_H

#include "UtilFrameRate.h"

inline bool operator<(const wxSize l, const wxSize& r) 
{ 
    return l.x < r.x || (l.x == r.x && l.y < r.y); 
}

namespace model {

class FileAnalyzer
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit FileAnalyzer(wxStrings fileNames, wxWindow* parent = nullptr);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getNumberOfFolders() const;
    int getNumberOfFiles() const;
    int getNumberOfMediaFiles() const;
    NodePtrs getNodes() const;

    wxString getFirstFolderName() const;

    wxSize getMostFrequentVideoSize() const;
    FrameRate getMostFrequentFrameRate() const;
    
    /// \return most requent audio type as a pair of framerate and number of channels
    std::pair<int,int> getMostFrequentAudioRate() const; 

    void addNodesToProjectView() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::shared_ptr<wxProgressDialog> mDialog;
    wxFileNames mFileNames; ///< Sorted list of input files.

    NodePtrs mNodes;

    wxSize mMostFrequentVideoSize;
    FrameRate mMostFrequentFrameRate;
    std::pair<int,int> mMostFrequentAudioRate;

    std::map<wxSize, int> mVideoSizeOccurrence;
    std::map<FrameRate, int> mFrameRateOccurrence;
    std::map<std::pair<int,int>, int> mAudioRateOccurrence;

    int mFolders;
    int mNumberOfMediaFiles;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void indexFolder(const wxFileName& dirName, bool recurse = true);
    FilePtr indexFile(const wxFileName& fileName);
    void updateProgressDialog();

};

} // namespace

#endif