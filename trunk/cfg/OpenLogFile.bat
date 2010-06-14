%HOMEDRIVE%
cd %TEMP%
FOR /F %%I IN ('DIR %TEMP%\Vidiot*.log /B /O:-D') DO copy /y %%I vidiot.log & GOTO theend

:theend
vidiot.log
