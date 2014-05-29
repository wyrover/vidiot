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
# along with Vidiot. If not, see <http:#www.gnu.org/licenses/>.

file (GLOB_RECURSE HtmlFiles ${PROJECT_SOURCE_DIR}/html/*.*)
add_custom_target (html COMMENT "Copying HTML files" SOURCES ${HtmlFiles})
source_group ("html\\about" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/html/about/.*)
source_group ("html\\help" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/html/help/.*)

macro (copy_html_files_to_target target)
  get_target_property (TargetLocation ${target} LOCATION)
  get_filename_component (TargetDir ${TargetLocation} PATH)
  add_custom_command (TARGET html DEPENDS ${HtmlFiles} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${TargetDir}/html)
  add_custom_command (TARGET html PRE_BUILD COMMAND echo "Html files copied to '${target}'")
  add_dependencies (${target} html)
endmacro ()

copy_html_files_to_target (main)
copy_html_files_to_target (testauto)
