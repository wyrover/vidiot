# Copyright 2013,2014 Eric Raijmakers.
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

if (NOT (EXISTS "${PROJECT_BINARY_DIR}/history.html" ) )
  message ( FATAL_ERROR "${PROJECT_BINARY_DIR}/history.html not found" )
endif()

file (GLOB_RECURSE HtmlFiles ${PROJECT_SOURCE_DIR}/html/*.*)
set( ${HtmlFiles} ${HtmlFiles} ${PROJECT_BINARY_DIR}/history.html )

macro (copy_html_files_to_target target)
  add_custom_command (TARGET ${target} DEPENDS ${HtmlFiles} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html  $<TARGET_FILE_DIR:${target}>/html)
  add_custom_command (TARGET ${target} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/history.html  $<TARGET_FILE_DIR:${target}>/html/about)
  add_custom_command (TARGET ${target} PRE_BUILD COMMAND echo Html files copied to '${target}')
endmacro ()
