d:
cd D:\Vidiot
set SOURCE=D:\Vidiot\vidiot_trunk
REM set PATH=%wxWidgets_ROOT_DIR%\lib\gcc_dll;%PATH%

REM del /s/q Build 
REM del /s/q BuildGcc

if NOT EXIST Build mkdir Build
cd Build
REM add --trace for more logging 
cmake -G "Visual Studio 9 2008" -Wdev --debug-output %SOURCE%

pause

if NOT EXIST BuildGcc mkdir BuildGcc
cd BuildGcc
REM add --trace for more logging 
cmake -G "CodeBlocks - MinGW Makefiles" -Wdev --debug-output %SOURCE%

cd %SOURCE%

pause
