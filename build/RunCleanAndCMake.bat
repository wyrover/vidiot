REM Go the the drive of the 'build' dir
call:EXTRACTDRIVE %0%
%DRIVE%
REM CD to the 'build' dir
cd %~dp0
RunCMake.bat REBUILD
