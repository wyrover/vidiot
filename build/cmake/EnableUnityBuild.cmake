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


# See
#    http://buffered.io/posts/the-magic-of-unity-builds/ ("The Magic of Unity Builds")
#    http://cheind.wordpress.com/2009/12/10/reducing-compilation-time-unity-builds/ ("Reducing Compilation Time: Unity Builds")
# On why this helps speed up compilation a lot (even when using a SSD).
# At the time I introduced this, build time was improved from 10:00 to 3:10 on my pc.
function (enable_unity_build FOLDER FILES OUTPUT_FILES)
    set (UNITY_CPP ${CMAKE_CURRENT_BINARY_DIR}/unity/unity_${FOLDER}.cpp)
    set (UNITY_TMP ${UNITY_CPP}_tmp)
    set_source_files_properties (${${FILES}} PROPERTIES HEADER_FILE_ONLY true)

    # Unity file is only recreated when it does not exist. Otherwise, each run of CMake triggers a full rebuild, since all the unity files get a new timestamp.
    file (WRITE ${UNITY_TMP} "// Unity Build generated by CMake\n")
    foreach (SOURCE_FILE ${${FILES}} )
      file (APPEND ${UNITY_TMP} "#include <${SOURCE_FILE}>\n")
    endforeach ()
    execute_process (COMMAND ${CMAKE_COMMAND} -E copy_if_different ${UNITY_TMP} ${UNITY_CPP})
    file (REMOVE ${UNITY_TMP})

    set (OUTPUT_FILES_LOCAL ${${OUTPUT_FILES}} ${UNITY_CPP})
    set (${OUTPUT_FILES} ${OUTPUT_FILES_LOCAL} PARENT_SCOPE) # Copy to scope of calling code
    source_group ("unity" FILES ${OUTPUT_FILES_LOCAL} )
endfunction ()
