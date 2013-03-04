#include "HelperApplication.h"

#include "Application.h"
#include "UtilLog.h"

namespace test {

void waitForIdle()
{
    static_cast<gui::Application*>(wxTheApp)->waitForIdle();
}

wxString randomString(int length)
{
    srand((unsigned)time(0));
    static const wxString alphanum = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";

    wxString result;
    for (int i = 0; i < length; ++i)
    {
        result += alphanum.GetChar(rand() % (sizeof(alphanum) - 1));
    }
    return result;
}

RandomTempDir::RandomTempDir(bool cleanup)
    : mFileName(wxFileName::GetTempDir(), "")
    , mCleanup(cleanup)
{
    mFileName.AppendDir(randomString(20) + wxDateTime::UNow().Format(wxT("%d%m%Y_%H%M%S_%l")));
    mFullPath = mFileName.GetLongPath();
    ASSERT(!wxDirExists(mFullPath))(mFullPath);
    mFileName.Mkdir();
    ASSERT(wxDirExists(mFullPath))(mFullPath);
}

RandomTempDir::~RandomTempDir()
{
    if (mCleanup)
    {
        bool removed = wxFileName::Rmdir( mFileName.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
        ASSERT(removed);
    }
}

wxFileName RandomTempDir::getFileName() const
{
    return mFileName;
}

void pause(int ms)
{
    VAR_WARNING(ms);
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
}

} // namespace