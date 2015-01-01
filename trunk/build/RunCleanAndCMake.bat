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

REM Go the the drive of the 'build' dir
set DRIVE=%~d0
%DRIVE%
REM CD to the 'build' dir
cd %~dp0
RunCMake.bat REBUILD
