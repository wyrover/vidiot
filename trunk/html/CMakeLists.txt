# Copyright 2013 Eric Raijmakers.
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


FILE(GLOB_RECURSE HtmlFiles ${PROJECT_SOURCE_DIR}/html/*.*)
add_custom_target(html COMMENT "Copying HTML files" SOURCES ${HtmlFiles})
SOURCE_GROUP("html\\about" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/html/about/.*)
SOURCE_GROUP("html\\help" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/html/help/.*)

MACRO(COPYHTMLFILESTOTARGET target)
    get_target_property(TargetLocation ${target} LOCATION)
    get_filename_component(TargetDir ${TargetLocation} PATH)
    add_custom_command(TARGET html DEPENDS ${HtmlFiles} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${TargetDir}/html)
    add_custom_command(TARGET html PRE_BUILD COMMAND echo "Html files copied to '${target}'" )
    add_dependencies(${target} html)
ENDMACRO(COPYHTMLFILESTOTARGET)
COPYHTMLFILESTOTARGET(main)
COPYHTMLFILESTOTARGET(testauto)
COPYHTMLFILESTOTARGET(testmanual)
