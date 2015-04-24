// Copyright 2013-2015 Eric Raijmakers.
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

#include "Test.h"

namespace test {

int RandomTempDir::sDirCount = 0;

wxString randomString(int length)
{
    srand((unsigned)time(0));
    static const std::string alphanum = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";

    wxString result;
    for (int i = 0; i < length; ++i)
    {
        result += alphanum.at(rand() % (sizeof(alphanum) - 1));
    }
    return result;
}

void RandomTempDir::makeDir()
{
    sDirCount++;
    wxString name = randomString(5) + wxString::Format("_%02d_",sDirCount) + wxDateTime::UNow().Format(wxT("%d%m%Y_%H%M%S_%l"));
    mFileName.AppendDir(name);
    mFullPath = mFileName.GetLongPath();
    ASSERT(!wxDirExists(mFullPath))(mFullPath);
    mFileName.Mkdir();
    ASSERT(wxDirExists(mFullPath))(mFullPath);
}

RandomTempDir::RandomTempDir(bool cleanup)
    : mFileName(wxFileName::GetTempDir(), "")
    , mCleanup(cleanup)
{
    mFileName.AppendDir("Vidiot");
    if (!wxDirExists(mFileName.GetLongPath()))
    {
        mFileName.Mkdir();
        ASSERT(wxDirExists(mFileName.GetLongPath()));
    }
    makeDir();
}

RandomTempDir::RandomTempDir(wxFileName path)
    : mFileName(path)
    , mCleanup(false)
{
    ASSERT(mFileName.Exists());
    ASSERT(mFileName.IsDir());
}

RandomTempDir::RandomTempDir(wxFileName parentDir, bool cleanup)
    : mFileName(parentDir)
    , mCleanup(cleanup)
{
    makeDir();
}

// static
RandomTempDirPtr RandomTempDir::generate(bool cleanup)
{
    return boost::make_shared<RandomTempDir>(cleanup);
}

RandomTempDirPtr RandomTempDir::generateSubDir()
{
    return boost::make_shared<RandomTempDir>(mFileName,mCleanup);
}

RandomTempDir::~RandomTempDir()
{
    if (mCleanup && wxDirExists(mFullPath))
    {
        VAR_DEBUG(mFileName.GetLongPath());
        bool removed = wxFileName::Rmdir( mFileName.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
        ASSERT(removed);
        ASSERT(!wxDirExists(mFileName.GetLongPath()));
    }
}

wxFileName RandomTempDir::getFileName() const
{
    return mFileName;
}

// see https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define stringify(x) dostringify(x)
#define dostringify(x) #x

wxFileName getSourceRoot()
{
#ifndef SOURCE_ROOT
#error "SOURCE_ROOT is not defined!"
#endif
    return wxFileName(stringify(SOURCE_ROOT),"");
}

wxFileName getTestPath()
{
    wxFileName result(getSourceRoot());
    result.AppendDir("test");
    ASSERT(result.IsDir())(result.GetFullPath());
    return result;
}

wxFileName getTestFilesPath(wxString subdir)
{
    wxFileName result(getTestPath());
    result.AppendDir(subdir);
    ASSERT(result.IsDir())(result.GetFullPath());
    ASSERT(result.DirExists())(result.GetFullPath());
    return result;
}

model::IPaths getListOfInputPaths(wxFileName path)
{
    // Not store model::FilePtr objects in the cache.
    // Otherwise, lifetime issues may (will...) arise.
    struct CachedPath : public model::IPath
    {
        explicit CachedPath(wxFileName path)
            : mFileName{ path }
        {}
        wxFileName getPath() const override { return mFileName; };
        wxFileName mFileName;
    };
	static std::map< wxString, model::IPaths > cache;
    
    model::IPaths result;
	wxString key = path.GetFullPath();
	auto it = cache.find(key);
	if (it != cache.end())
	{
		result = it->second;
	}
	else
	{
        for (model::IPathPtr path : GetSupportedFiles(path))
        {
            result.push_back(boost::make_shared<CachedPath>(path->getPath()));
        }
		cache[key] = result;
	}
	return result;
}

wxStrings getListOfInputPathsAsStrings(wxFileName path)
{
    wxStrings result;
    for (auto f : getListOfInputPathsAsFileNames(path))
    {
        result.push_back(f.GetFullPath());
    }
    return result;
}

wxFileNames getListOfInputPathsAsFileNames(wxFileName path)
{
    wxFileNames result;
    for (auto f : getListOfInputPaths(path))
    {
        result.push_back(f->getPath());
    }
    return result;
}

wxFileName getStillImagePath()
{
    wxFileName result = getTestPath();
    result.AppendDir("filetypes_image");
    ASSERT(result.IsDir())(result.GetFullPath());
    ASSERT(result.DirExists())(result.GetFullPath());
    result.SetFullName("Laney -6th best amp.jpg");
    return result;
}

wxString getFileContents(wxFileName path)
{
    ASSERT(path.Exists());
    wxFile file(path.GetFullPath());
    ASSERT(file.IsOpened());
    wxString contents;
    file.ReadAll(&contents);
    return contents;
}

wxString getSavedFileContents(wxFileName path)
{
	wxString contents(getFileContents(path));
	int i = contents.length();
	//wxRegEx reRemoveModifiedDates("<mLastModified>[[:digit:]]+</mLastModified>");
	//int removed = reRemoveModifiedDates.ReplaceAll(&contents,"<mLastModified>0000000000</mLastModified>");
	//ASSERT_MORE_THAN_ZERO(removed);
	int j = contents.length();
	return contents;
}

void OpenFileExplorer(wxFileName dir)
{
    ASSERT(dir.IsDir());
    util::thread::RunInMainAndWait([dir]()
    {
        wxString cmd;
        cmd << "explorer " << dir.GetFullPath();
        ::wxExecute(cmd, wxEXEC_ASYNC, NULL);
    });
}

} // namespace
