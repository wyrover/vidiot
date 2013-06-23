#include "HelperFileSystem.h"

#include "UtilLog.h"
#include <wx/string.h>

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
        bool removed = wxFileName::Rmdir( mFileName.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
        ASSERT(removed);
        ASSERT(!wxDirExists(mFileName.GetLongPath()));
    }
}

wxFileName RandomTempDir::getFileName() const
{
    return mFileName;
}

wxFileName getTestFilesPath()
{
    wxFileName result = wxFileName(SOURCE_ROOT,"");
    result.AppendDir("test");
    result.AppendDir("input");
    ASSERT(result.IsDir());
    ASSERT(result.DirExists());
    return result;
}

model::IPaths getListOfInputFiles()
{
    return getSupportedFiles(getTestFilesPath());
}

} // namespace