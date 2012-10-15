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
    RandomTempDir();
    virtual ~RandomTempDir();
    wxFileName getFileName() const;
private:
    wxFileName mFileName;
};

/// Delay the test for 60 seconds to allow using the GUI (debugging)
void pause(int ms = 60000);

} // namespace

#endif // HELPER_APPLICATION_H