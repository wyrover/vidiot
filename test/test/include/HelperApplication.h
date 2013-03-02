#ifndef HELPER_APPLICATION_H
#define HELPER_APPLICATION_H

#include <wx/string.h>

namespace test {

/// Wait until the main application loop signals Idle via a idle event.
/// If the loop is already idle, an extra idle event is triggered.
void waitForIdle();

/// Generate a random string using alphanumeric characters of size length
wxString randomString(int length = 8);

class RandomTempDir
{
public:
    explicit RandomTempDir(bool cleanup = true);
    virtual ~RandomTempDir();
    wxFileName getFileName() const;
private:
    wxString mFullPath; ///< Usefull when debugging (with wxFileName  it's not easy to extract the full path)
    wxFileName mFileName;
    bool mCleanup;
};

/// Delay the test for 60 seconds to allow using the GUI (debugging)
void pause(int ms = 60000);

} // namespace

#endif // HELPER_APPLICATION_H