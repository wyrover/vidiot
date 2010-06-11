set WXWIN=D:\VideoApp\InstalledLibraries\wxWidgets-2.9.0
cd %WXWIN%\build\msw
mingw32-make -f makefile.gcc SHARED=1 UNICODE=1 BUILD=release

set PATH=%WXWIN%\lib\gcc_dll;%PATH%
#CMake: -DwxWidgets_LIB_DIR=%WXWIN%/lib/gcc_dll -DwxWidgets_CONFIGURATION=mswu -DENABLE_MIX_CXX=ON

pause