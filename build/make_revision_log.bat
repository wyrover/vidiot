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
"%SAXON_PATH%\Transform.exe" -s:%XML_SOURCE% -xsl:%VIDIOT_DIR%\vidiot_trunk\build\make_revision_log.xslt -o:"%VIDIOT_DIR%\Build\MSVC\VersionHistory.txt"

:end
