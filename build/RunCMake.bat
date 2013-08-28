set STARTTIME=%TIME%

goto BEGIN

REM ============================== SUBROUTINES ==============================

:EXTRACTDRIVE
set DRIVE=%~d1
goto:eof

:NOVAR
echo Define system variable VIDIOT_DIR first
echo Press any key to exit
pause
exit


REM ============================== BEGIN ==============================
:BEGIN

REM Go the the drive of the 'build' dir
call:EXTRACTDRIVE %0%
%DRIVE%

REM CD to the 'build' dir
cd %~dp0

REM Find the source dir
pushd ..
set SOURCE=%CD%
echo Sources are in %SOURCE%
popd


if NOT DEFINED VIDIOT_DIR goto NOVAR
call:EXTRACTDRIVE %VIDIOT_DIR%
%DRIVE%
cd %VIDIOT_DIR%

if NOT DEFINED VIDIOT_BUILD SET VIDIOT_BUILD=%VIDIOT_DIR%\Build
call:EXTRACTDRIVE %VIDIOT_BUILD%
SET VIDIOT_BUILD_DRIVE=%DRIVE%




REM ============================== CLEAN ==============================
:CLEAN
if "%1%"=="" goto PREPARE

del /s/q/f %VIDIOT_BUILD%\*
rd /s/q  %VIDIOT_BUILD%
mkdir %VIDIOT_BUILD%



REM ============================== PREPARE ==============================
:PREPARE

set BUILD_DIR=%VIDIOT_BUILD%
%VIDIOT_BUILD_DRIVE%
cd %BUILD_DIR%
if NOT EXIST MSVC mkdir MSVC
if NOT EXIST GCCD mkdir GCCD
if NOT EXIST GCCR mkdir GCCR




REM ======================= UPDATE VERSION INFO ========================
if NOT "%1%"=="DELIVER" goto BUILD

REM Call svn Update to ensure that the about box and the logging show the proper revision
cd %SOURCE%
"C:\Program Files\TortoiseSVN\bin\svn.exe" update

REM Generate revision log file
call %VIDIOT_DIR%\vidiot_trunk\build\make_revision_log.bat




REM ============================== BUILD ==============================
:BUILD
REM === FIND BOOST ====
REM o-d: always use newest version
%DRIVE%
cd %VIDIOT_DIR%
dir boost* /b /o-d > %TEMP%\boostdir.txt
set /p BOOSTDIR=<%TEMP%\boostdir.txt
set BOOSTVERSION=%BOOSTDIR:boost_=%
set FOUND_BOOST_VERSION="%BOOSTVERSION:_=.%"
set BOOST_ROOT=%VIDIOT_DIR%\%BOOSTDIR%
set BOOST_ROOT=%BOOST_ROOT:\=/%


REM === CREATE SOLUTION ====
REM del /s/q Build 
set wxWidgets_ROOT_DIR=%VIDIOT_DIR%\wxwidgets_trunk

%VIDIOT_BUILD_DRIVE%
cd %VIDIOT_BUILD%\..
if NOT EXIST Build mkdir Build

REM add --trace to a cmake line for more logging 

if EXIST "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"

cd %BUILD_DIR%\MSVC
set OUTTYPE="Visual Studio 9 2008"
if EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0" set OUTTYPE="Visual Studio 10"
if EXIST "%ProgramFiles%\Microsoft Visual Studio 10.0" set OUTTYPE="Visual Studio 10"
cmake -G %OUTTYPE% -Wdev --debug-output %SOURCE%
cmake -LAH  %SOURCE% > CMakeVariables.txt

REM cd %BUILD_DIR%\GCCD
REM cmake -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="DEBUG" -Wdev --debug-output %SOURCE%

REM cd %BUILD_DIR%\GCCR
REM cmake -G "CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="RELEASE" -Wdev --debug-output %SOURCE%



REM ============================== DELIVER ==============================
:DELIVER
if NOT "%1%"=="DELIVER" goto END

cd %BUILD_DIR%\MSVC
"C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv" Vidiot.sln /Build RelWithDebInfo /project PACKAGE 
for %%i in (Vidiot*.exe) do start "" /b "%%i"

:END
set ENDTIME=%TIME%

@echo off
echo STARTTIME: %STARTTIME%
echo ENDTIME: %ENDTIME%
set /A STARTTIME=(1%STARTTIME:~0,2%-100)*360000 + (1%STARTTIME:~3,2%-100)*6000 + (1%STARTTIME:~6,2%-100)*100 + (1%STARTTIME:~9,2%-100)
set /A ENDTIME=(1%ENDTIME:~0,2%-100)*360000 + (1%ENDTIME:~3,2%-100)*6000 + (1%ENDTIME:~6,2%-100)*100 + (1%ENDTIME:~9,2%-100)
set /A DURATION=%ENDTIME%-%STARTTIME%
set /A DURATIONH=%DURATION% / 360000
set /A DURATIONM=(%DURATION% - %DURATIONH%*360000) / 6000
set /A DURATIONS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000) / 100
set /A DURATIONHS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000 - %DURATIONS%*100)
if %DURATIONH% LSS 10 set DURATIONH=0%DURATIONH%
if %DURATIONM% LSS 10 set DURATIONM=0%DURATIONM%
if %DURATIONS% LSS 10 set DURATIONS=0%DURATIONS%
if %DURATIONHS% LSS 10 set DURATIONHS=0%DURATIONHS%
echo DURATION: %DURATIONH%:%DURATIONM%:%DURATIONS%.%DURATIONHS%

pause
exit



:NOVAR
echo "Define VIDIOT_DIR first"
exit
