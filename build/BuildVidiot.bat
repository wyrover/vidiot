@echo off

REM Copyright 2013-2015 Eric Raijmakers.
REM
REM This file is part of Vidiot.
REM
REM Vidiot is free software: you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation, either version 3 of the License, or
REM (at your option) any later version.
REM
REM Vidiot is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with Vidiot. If not, see <http://www.gnu.org/licenses />.

@SETLOCAL enableextensions enabledelayedexpansion

set STARTTIME=%TIME: =0%

set VSDIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0
set OUTTYPE=Visual Studio 14 2015
if not exist "%VSDIR%" goto:NOVS

goto BEGIN

REM ============================== SUBROUTINES ==============================

:EXTRACTDRIVE
set DRIVE=%~d1
goto:eof

:CREATESHORTCUT
((
  echo Set oWS = WScript.CreateObject^("WScript.Shell"^) 
  echo sLinkFile = oWS.ExpandEnvironmentStrings^("%%HOMEDRIVE%%%%HOMEPATH%%\Desktop\V_%2%.lnk"^)
  echo Set oLink = oWS.CreateShortcut^(sLinkFile^) 
  echo oLink.TargetPath = oWS.ExpandEnvironmentStrings^(%1%^)
  echo oLink.Arguments = "%2%" 
  echo oLink.Save
)1>%TEMP%\CreateShortcut.vbs
cscript //nologo %TEMP%\CreateShortcut.vbs
)1>>%TEMP%\CreateShortcut.log 2>>&1
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


if NOT DEFINED VIDIOT_DIR goto :NOVAR
call:EXTRACTDRIVE %VIDIOT_DIR%
%DRIVE%
cd %VIDIOT_DIR%

if NOT DEFINED VIDIOT_BUILD SET VIDIOT_BUILD=%VIDIOT_DIR%\Build
call:EXTRACTDRIVE %VIDIOT_BUILD%
SET VIDIOT_BUILD_DRIVE=%DRIVE%



REM ============================== CREATE DESKTOP SHORTCUTS ============================== 

call:CREATESHORTCUT %0% CMAKE
call:CREATESHORTCUT %0% REBUILD
call:CREATESHORTCUT %0% DELIVER



REM ============================== CLEAN ==============================
:CLEAN
if "%1%"=="" goto PREPARE
if "%1%"=="CMAKE" goto PREPARE

del /s/q/f %VIDIOT_BUILD%\*
rd /s/q  %VIDIOT_BUILD%
mkdir %VIDIOT_BUILD%



REM ============================== PREPARE ==============================
:PREPARE

%VIDIOT_BUILD_DRIVE%
cd %VIDIOT_DIR%
if NOT EXIST Build mkdir Build

cd %VIDIOT_BUILD%
if "%1%"=="CMAKE" goto BUILD



REM ======================= UPDATE VERSION INFO ========================

REM === Update svn version ===
cd %SOURCE%
"C:\Program Files\TortoiseSVN\bin\svn.exe" update

REM === Generate revision log xml file ===
set XML_SOURCE=%VIDIOT_BUILD%\Changelog.xml
if not exist %XML_SOURCE% (
  "c:\Program Files\TortoiseSVN\bin\svn.exe" log %VIDIOT_DIR%\vidiot_trunk --xml -r 1200:BASE > %XML_SOURCE%
)

REM ==== Find SAXON ====
set SAXON_PATH=
for /R "C:\Program Files\Saxonica" /D %%a in (*) do if exist "%%a\Transform.exe" set SAXON_PATH=%%a& goto:foundsaxon
echo "Could not find Saxon in C:\Program Files"
pause
exit

REM === Generate changelog files ===
:foundsaxon
if not exist "%VIDIOT_BUILD%\Changelog.txt" "%SAXON_PATH%\Transform.exe" -s:%XML_SOURCE% -xsl:%VIDIOT_DIR%\vidiot_trunk\build\Changelog_txt.xslt -o:"%VIDIOT_BUILD%\Changelog.txt"
if not exist "%VIDIOT_BUILD%\changelog.html" "%SAXON_PATH%\Transform.exe" -s:%XML_SOURCE% -xsl:%VIDIOT_DIR%\vidiot_trunk\build\Changelog_htm.xslt -o:"%VIDIOT_BUILD%\changelog.html"
copy /Y "%VIDIOT_BUILD%\Changelog.txt" "%VIDIOT_BUILD%\README.txt"
           
REM ==== Generate Known Problems file ====

C:\Python27\python.exe "%VIDIOT_DIR%\vidiot_trunk\build\make_knownproblems.py" "%VIDIOT_BUILD%\knownproblems.html" Windows



REM ============================== BUILD ==============================
:BUILD

REM === UPDATE TRANSLATIONS FILES ===

REM Remove and ~ files
dir /s /o /b %SOURCE%\*~ > %TEMP%\files.txt
for /f "delims=" %%f in (%TEMP%\files.txt) do del "%%f"

set LANGTOOLSDIR=C:\Program Files (x86)\Poedit\Gettexttools\bin
set LANGDIR=%SOURCE%\lang
pushd %SOURCE%
dir /b /s *.cpp *.h > "%TMP%/files.txt"
popd
if not exist "%LANGTOOLSDIR%" goto:NOPO
echo Updating "%LANGDIR%\vidiot.pot" template
"%LANGTOOLSDIR%\xgettext.exe" -f "%TMP%/files.txt" --output=%LANGDIR%\vidiot.pot --keyword=_

REM Update all translations 
for /f "tokens=*" %%L in ('dir /b/o "%%LANGDIR%%"') do (
    if exist "%LANGDIR%\%%L\vidiot.po" (
        echo Updating language %%L

        echo Updating "%LANGDIR%\%%L\vidiot.po" from "%LANGDIR%\vidiot.pot"
        "%LANGTOOLSDIR%\msgmerge.exe" -U "%LANGDIR%\%%L\vidiot.po" "%LANGDIR%\vidiot.pot"
        
        echo Compiling "%LANGDIR%\%%L\vidiot.po"
        "%LANGTOOLSDIR%\msgfmt.exe" "%LANGDIR%\%%L\vidiot.po" --output-file="%LANGDIR%\%%L\vidiot.mo"

        if exist "%VIDIOT_DIR%\wxWidgets\locale\%%L.po" (
		
            echo Updating wxwidgets %%L.po into "%LANGDIR%\%%L\vidiotwx.po"
            copy "%VIDIOT_DIR%\wxWidgets\locale\%%L.po" "%LANGDIR%\%%L\vidiotwx.po"

            echo Compiling "%LANGDIR%\%%L\vidiotwx.po"
            "%LANGTOOLSDIR%\msgfmt.exe" "%LANGDIR%\%%L\vidiotwx.po" --output-file="%LANGDIR%\%%L\vidiotwx.mo"
        )
    )
)




REM === FIND BOOST ====
REM o-d: always use newest version
%VIDIOT_BUILD_DRIVE%
cd %VIDIOT_DIR%
dir boost* /b /o-d > %TEMP%\boostdir.txt
set /p BOOSTDIR=<%TEMP%\boostdir.txt
set BOOSTVERSION=%BOOSTDIR:boost_=%
set FOUND_BOOST_VERSION="%BOOSTVERSION:_=.%"
set BOOST_ROOT=%VIDIOT_DIR%\%BOOSTDIR%
set BOOST_ROOT=%BOOST_ROOT:\=/%

REM === CREATE SOLUTION ====
set wxWidgets_ROOT_DIR=%VIDIOT_DIR%\wxwidgets

REM add --trace to a cmake line for more logging 

echo call "%VSDIR%\VC\bin\vcvars32.bat"

cd %VIDIOT_BUILD%
cmake -G "%OUTTYPE%" -Wno-dev --debug-output %SOURCE%
REM -Wdev 





REM ============================== DELIVER ==============================
:DELIVER
if NOT "%1%"=="DELIVER" goto END

cd %VIDIOT_BUILD%
"%VSDIR%\Common7\IDE\devenv" Vidiot.sln /Build RelWithDebInfo /project PACKAGE 
for %%i in (Vidiot*.exe) do start "" /b "%%i"

:END
set ENDTIME=%TIME: =0%

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
pause
exit

:NOPO
echo "Install poEdit"
pause
exit

:NOVS
echo %VSDIR% not found
pause
exit