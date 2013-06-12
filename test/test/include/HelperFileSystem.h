#ifndef HELPER_FILE_SYSTEM_H
#define HELPER_FILE_SYSTEM_H

namespace test {

class RandomTempDir;
typedef boost::shared_ptr<RandomTempDir> RandomTempDirPtr;

class RandomTempDir : boost::noncopyable
{
public:

    explicit RandomTempDir(bool cleanup = true);
    RandomTempDir(wxFileName parentDir, bool cleanup); ///< Make subdir
    virtual ~RandomTempDir();

    static RandomTempDirPtr generate(bool cleanup = true);
    RandomTempDirPtr generateSubDir();

    wxFileName getFileName() const;

private:

    void makeDir();

    wxString mFullPath; ///< Useful when debugging (with wxFileName  it's not easy to extract the full path)
    wxFileName mFileName;
    bool mCleanup;

    static int sDirCount;
};

/// Get the path to the folder containing the test input files
/// \return path to input files
wxFileName getTestFilesPath();

/// Get the input files in the test input folder
/// \return list of paths to input files
model::IPaths getListOfInputFiles();

} // namespace

#endif // HELPER_FILE_SYSTEM_H