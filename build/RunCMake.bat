goto BEGIN

:EXTRACTDRIVE
set DRIVE=%~d1
goto:eof

:NOVAR
echo Define system variable VIDIOT_DIR first
echo Press any key to exit
pause
exit



REM ==== BEGIN ====
:BEGIN
echo %CD%
if NOT DEFINED VIDIOT_DIR goto NOVAR
call:EXTRACTDRIVE %VIDIOT_DIR%
%DRIVE%
cd %VIDIOT_DIR%



REM === FIND BOOST ====
REM o-d: always use newest version
%DRIVE%
cd %VIDIOT_DIR%
dir boost* /b /o-d > %TEMP%\boostdir.txt
set /p BOOSTDIR=<%TEMP%\boostdir.txt
set BOOSTVERSION=%BOOSTDIR:boost_=%
set FOUND_BOOST_VERSION="%BOOSTVERSION:_=.%"
set BOOST_ROOT=%VIDIOT_DIR%\BOOSTDIR



REM === CREATE SOLUTION ====
REM del /s/q Build 
set wxWidgets_ROOT_DIR=%VIDIOT_DIR%\wxwidgets_trunk
set SOURCE=%VIDIOT_DIR%\vidiot_trunk

cd %VIDIOT_DIR%
if NOT EXIST Build mkdir Build
set BUILD_DIR=%VIDIOT_DIR%\Build
cd Build
if NOT EXIST MSVC mkdir MSVC
if NOT EXIST GCCD mkdir GCCD
if NOT EXIST GCCR mkdir GCCR

REM add --trace to a cmake line for more logging 

if EXIST "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"

cd %BUILD_DIR%\MSVC
set OUTTYPE="Visual Studio 9 2008"
if EXIST "%ProgramFiles%\Microsoft Visual Studio 10.0" set OUTTYPE="Visual Studio 10"
cmake -G %OUTTYPE% -Wdev --debug-output %SOURCE%
cmake -LAH  %SOURCE% > CMakeVariables.txt
pause
exit

cd %BUILD_DIR%\GCCD
cmake -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="DEBUG" -Wdev --debug-output %SOURCE%

cd %BUILD_DIR%\GCCR
cmake -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="RELEASE" -Wdev --debug-output %SOURCE%

