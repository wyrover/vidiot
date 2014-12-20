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

file (GLOB IconFiles ${PROJECT_SOURCE_DIR}/images/*?.???)

macro (copy_images_to_target target)
    add_custom_command (TARGET ${target} DEPENDS ${IconFiles} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/images $<TARGET_FILE_DIR:${target}>/images)
    add_custom_command (TARGET ${target} PRE_BUILD COMMAND echo Images copied to '${target}')
endmacro ()
