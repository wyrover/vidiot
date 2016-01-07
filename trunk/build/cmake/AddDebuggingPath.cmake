# Copyright 2013-2016 Eric Raijmakers.
#
# This file is part of Vidiot.
#
# Vidiot is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vidiot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vidiot. If not, see <http:#www.gnu.org/licenses/>.

# Fill this variable with extra dll paths required for running the application
# in a debugging context. The variable is used to set MSVC debugging paths.
# This avoids having to change the system path for running with dlls from E.g.
# ffmpeg, boost, and portaudio in the debugger.
set (DEBUGGING_PATH "")
set (MSVC_USER_TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/build/template.vcxproj.user)
macro (add_debugging_path dllpath)
  if (MSVC)
    set (DEBUGGING_PATH ${DEBUGGING_PATH};${dllpath})
  endif ()
endmacro ()
macro (copy_msvc_project_user_file projectname)
  configure_file (${MSVC_USER_TEMPLATE_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${projectname}.vcxproj.user @ONLY)
endmacro ()
