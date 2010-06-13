%HOMEDRIVE%
cd %TEMP%
FOR /F %%I IN ('DIR %TEMP%\Vidiot*.log /B /O:-D') DO %%I & EXIT


