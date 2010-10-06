set VIDIOT_DIR=D:\Vidiot
set wxWidgets_ROOT_DIR=%VIDIOT_DIR%\wxwidgets_trunk

d:
cd %VIDIOT_DIR%
set SOURCE=%VIDIOT_DIR%\vidiot_trunk

REM del /s/q Build 

cd %VIDIOT_DIR%
if NOT EXIST Build mkdir Build
set BUILD_DIR=%VIDIOT_DIR%\Build
cd Build
if NOT EXIST MSVC mkdir MSVC
if NOT EXIST GCCD mkdir GCCD
if NOT EXIST GCCR mkdir GCCR

REM add --trace to a cmake line for more logging 

cd %BUILD_DIR%\MSVC
cmake -G "Visual Studio 9 2008" -Wdev --debug-output %SOURCE%

pause

cd %BUILD_DIR%\GCCD
cmake -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="DEBUG" -Wdev --debug-output %SOURCE%

pause

cd %BUILD_DIR%\GCCR
cmake -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="RELEASE" -Wdev --debug-output %SOURCE%

pause
