#include "TestUtilPath.h"

#include "UtilPath.h"
#include "HelperFileSystem.h"
#include "UtilLog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUtilPath::testEqualsAndIsParentOf()
{
    StartTestSuite();

    ASSERT( util::path::equals( wxFileName("C:\\test"), wxString("C:\\test") ) );
    ASSERT( util::path::equals( wxFileName("C:/test"),  wxString("C:\\test") ) );
    ASSERT( util::path::equals( wxFileName("C:/test"),  wxString("C:/test") ) );
    ASSERT( util::path::equals( wxFileName("C:\\test"), wxString("C:\\test\\") ) );
    ASSERT( util::path::equals( wxFileName("C:\\test"), wxString("C:/test/") ) );
    ASSERT( util::path::equals( wxFileName("C:\\test"), wxString("C:/test/") ) );

    ASSERT( util::path::equals( wxString("C:\\test"),   wxFileName("C:\\test") ) );
    ASSERT( util::path::equals( wxString("C:\\test"),   wxFileName("C:/test") ) );
    ASSERT( util::path::equals( wxString("C:/test"),    wxFileName("C:/test") ) );
    ASSERT( util::path::equals( wxString("C:\\test\\"), wxFileName("C:\\test") ) );
    ASSERT( util::path::equals( wxString("C:/test/"),   wxFileName("C:\\test")  ) );
    ASSERT( util::path::equals( wxString("C:/test/"),   wxFileName("C:\\test") ) );

    RandomTempDir tempdir;

    wxString shortPath = tempdir.getFileName().GetShortPath();
    wxString longPath = tempdir.getFileName().GetLongPath();
    ASSERT( !shortPath.IsSameAs(longPath) ); // If this fails all the tests below actually test nothing
    ASSERT( util::path::equals( shortPath, longPath ) );
    ASSERT( util::path::equals( wxFileName(shortPath), wxString(longPath) ) );
    ASSERT( util::path::equals( wxFileName(shortPath, "file", "ext"), wxFileName(longPath, "file", "ext") ) );

    wxFileName path = tempdir.getFileName();
    wxFileName root = path;
    wxArrayString paths = path.GetDirs();
    ASSERT_MORE_THAN(paths.GetCount(), 3); // If this fails then the TEMP dir is probably too 'short'
    root.RemoveLastDir();
    root.RemoveLastDir();
    root.RemoveLastDir();
    wxString path1 = root.GetFullPath() + "\\test\\dir\\dir\\file.ext";
    wxString path2 = path.GetFullPath() +  "..\\..\\..\\test\\dir\\dir\\file.ext";
    wxFileName fn1 = root;
    fn1.AppendDir("test");
    fn1.AppendDir("dir");
    fn1.AppendDir("dir");
    fn1.SetFullName("file.ext");
    wxFileName fn2  = path;
    fn2.AppendDir("..");
    fn2.AppendDir("..");
    fn2.AppendDir("..");
    fn2.AppendDir("test");
    fn2.AppendDir("dir");
    fn2.AppendDir("dir");
    fn2.SetFullName("file.ext");
    wxFileName fnWrong = path;
    fnWrong.AppendDir("..");
    fnWrong.AppendDir("..");
    fnWrong.AppendDir("..");
    fnWrong.AppendDir("test");
    fnWrong.AppendDir("dir");
    fnWrong.SetFullName("file.ext");

    ASSERT( util::path::equals( path1, path2 ) );
    ASSERT( util::path::equals( path1, fn1 ) )(path1)(fn1);
    ASSERT( util::path::equals( fn1, path1 ) );
    ASSERT( util::path::equals( fn2, path2 ) )(fn2)(path2);
    ASSERT( util::path::equals( path2, fn2 ) );
    ASSERT( util::path::equals( fn1, fn2 ) );

    ASSERT( !util::path::equals( fn1, fnWrong ) );

    ASSERT( util::path::isParentOf( root.GetFullPath(), path1 ) )(root)(path1);
    ASSERT( util::path::isParentOf( root, path2 ) );
    ASSERT( util::path::isParentOf( root.GetFullPath(), fn1 ) );
    ASSERT( util::path::isParentOf( root, fn2 ) );

    ASSERT( !util::path::isParentOf( root, root ) );
    ASSERT( !util::path::isParentOf( root, root.GetFullPath() ) );
    ASSERT( !util::path::isParentOf( root.GetFullPath(), root ) );
    ASSERT( !util::path::isParentOf( root.GetFullPath(), root.GetFullPath() ) );

    ASSERT( !util::path::isParentOf( path1, root ) );
    ASSERT( !util::path::isParentOf( path2, root.GetFullPath() ) );
    ASSERT( !util::path::isParentOf( fn1, root ) );
    ASSERT( !util::path::isParentOf( fn2, root.GetFullPath() ) );
}

} // namespace