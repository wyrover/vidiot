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

FILE(GLOB installfiles ${PROJECT_SOURCE_DIR}/install/*.txt)
SOURCE_GROUP("install" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/install/*.*)

SOURCE_GROUP("exe" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/main/src/.* FILES CMakeLists.txt ../icons/icons.rc)

# icons.rc is required to embed the icon file in the executable
ADD_EXECUTABLE(main WIN32 src/main.cpp ${installfiles} ../icons/icons.rc)
SET_TARGET_PROPERTIES(main PROPERTIES OUTPUT_NAME Vidiot)
TARGET_LINK_LIBRARIES(main vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

COPY_MSVC_PROJECT_USER_FILE(main)

INSTALL(TARGETS main COMPONENT Vidiot DESTINATION .)

add_custom_command(TARGET main DEPENDS vidiot PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_CURRENT_BINARY_DIR}/Debug/html)

# Copy icons to output folder
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Debug/icons)
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Release/icons)
