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
# along with Vidiot. If not, see <http:#www.gnu.org/licenses/>.

macro (AddTests TestProjectName)
  include_directories (AFTER
    ${CMAKE_CURRENT_SOURCE_DIR}/../test/include
    include
  )

  set (TESTDIR ${CMAKE_CURRENT_SOURCE_DIR})
  file (GLOB TestHeaders ${TESTDIR}/*/*.h)
  file (GLOB TestSources ${TESTDIR}/*/*.cpp)

  enable_unity_build (${TestProjectName} TestSources TEST_AUTO_UNITY_CPP_FILES)

  AddCxxTest (${TestProjectName} ${TEST_AUTO_UNITY_CPP_FILES} ${TestHeaders} ${TestSources})


  if (UNIX)
    set_target_properties (${TestProjectName} PROPERTIES OUTPUT_NAME ${TestProjectName}.run) # Rename to avoid having the same name as the folder
  endif ()

  source_group ("generated" REGULAR_EXPRESSION ${TestRunnerDirectory}/.*)
  source_group ("src" REGULAR_EXPRESSION ${TESTDIR}/src/.*)
  source_group ("include" REGULAR_EXPRESSION ${TESTDIR}/include/.*)

  target_link_libraries (${TestProjectName} test vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

  copy_msvc_project_user_file (${TestProjectName})

  copy_html_files_to_target (${TestProjectName})
  copy_images_to_target (${TestProjectName})

endmacro ()

