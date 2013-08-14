REM Go the the drive of the 'build' dir
set DRIVE=%~d0
%DRIVE%
REM CD to the 'build' dir
cd %~dp0
RunCMake.bat DELIVER
