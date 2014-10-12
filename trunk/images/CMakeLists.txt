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

file (GLOB IconFiles ${PROJECT_SOURCE_DIR}/icons/*?.???)
add_custom_target (icons COMMENT "Copying ICONS" SOURCES ${IconFiles})
source_group ("icons"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/icons/*.*)

macro (copy_icons_to_target target)
    add_custom_command (TARGET icons DEPENDS ${IconFiles} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons $<TARGET_FILE_DIR:${target}>/icons)
    add_custom_command (TARGET icons PRE_BUILD COMMAND echo "Icons copied to '${target}'")
endmacro ()

copy_icons_to_target(main)
copy_icons_to_target(testauto)
