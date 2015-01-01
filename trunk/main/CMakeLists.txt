# Copyright 2013-2015 Eric Raijmakers.
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

FILE(GLOB installfiles ${PROJECT_SOURCE_DIR}/install/*.txt)
SOURCE_GROUP("install" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/install/*.*)

SOURCE_GROUP("exe" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/main/src/.* FILES CMakeLists.txt ../images/images.rc)

LINK_DIRECTORIES(
    ${FFMPEG_LIBRARY_DIR}
    ${Boost_LIBRARY_DIRS}
    ${PORTAUDIO_LIBRARY_DIR}
    ${SOUNDTOUCH_LIBRARY_DIR}
    ${VLD_LIBRARIES_DIR}
)

# images.rc is required to embed the icon file in the executable
ADD_EXECUTABLE(main WIN32 src/Main.cpp ${installfiles} ../images/images.rc)
SET_TARGET_PROPERTIES(main PROPERTIES OUTPUT_NAME Vidiot)
TARGET_LINK_LIBRARIES(main vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

reuse_precompiled_header (main "${CMAKE_CURRENT_SOURCE_DIR}/../pch" "Precompiled")

copy_msvc_project_user_file(main)

INSTALL(TARGETS main COMPONENT Vidiot DESTINATION .)

add_custom_command(TARGET main DEPENDS vidiot)

copy_html_files_to_target (main)
copy_images_to_target (main)
