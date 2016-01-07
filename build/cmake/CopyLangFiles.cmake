# Copyright 2015-2016 Eric Raijmakers.
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
# along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

file (GLOB_RECURSE LangFiles ${PROJECT_SOURCE_DIR}/lang/*.*)
set( ${LangFiles} ${LangFiles} )

macro (copy_lang_files_to_target target)
  add_custom_command (TARGET ${target} DEPENDS ${LangFiles} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/lang  $<TARGET_FILE_DIR:${target}>/lang)
  add_custom_command (TARGET ${target} PRE_BUILD COMMAND echo Lang files copied to '${target}')
endmacro ()
