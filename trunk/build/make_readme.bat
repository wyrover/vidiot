REM Copyright 2013,2014 Eric Raijmakers.
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

set XML_SOURCE=%TEMP%\revisionlog.xml

REM ==== UPDATE SVN AND GET REVISION LOG ====
"C:\Program Files\TortoiseSVN\bin\svn.exe" update %VIDIOT_DIR%\vidiot_trunk
"c:\Program Files\TortoiseSVN\bin\svn.exe" log %VIDIOT_DIR%\vidiot_trunk --xml -r 1200:BASE > %XML_SOURCE%

REM ==== FIND SAXON ====
@echo off
set SAXON_PATH=
for /R "C:\Program Files\Saxonica" /D %%a in (*) do if exist "%%a\Transform.exe" set SAXON_PATH=%%a& goto:foundsaxon
echo "Could not find Saxon in C:\Program Files"
pause
exit
goto:end

:foundsaxon
"%SAXON_PATH%\Transform.exe" -s:%XML_SOURCE% -xsl:%VIDIOT_DIR%\vidiot_trunk\build\make_readme.xslt -o:"%VIDIOT_DIR%\Build\MSVC\README.txt"

:end
