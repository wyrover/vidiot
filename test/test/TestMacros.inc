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

# todo rename to .cmake file (and place in 'build?')
set(TESTDIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(../test/CxxTestMacros.inc)

INCLUDE_DIRECTORIES(AFTER
#    ${CMAKE_CURRENT_SOURCE_DIR}/../pch/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../test/include
    include
)

macro(AddTests TestProjectName)
    file(GLOB TestHeaders ${TESTDIR}/*/*.h)
    file(GLOB TestSources ${TESTDIR}/*/*.cpp)

    enable_unity_build(${TestProjectName} TestSources TEST_AUTO_UNITY_CPP_FILES)

    AddCxxTest(${TestProjectName} ${TEST_AUTO_UNITY_CPP_FILES} ${TestHeaders} ${TestSources})

    reuse_precompiled_header (${TestProjectName} "${CMAKE_CURRENT_SOURCE_DIR}/../pch" "PrecompiledTest")

    if (MSVC)
  #          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI${pch_file} ")
  #          #set_target_properties(${TestProjectName} PROPERTIES COMPILE_FLAGS "/Yu${pch_file} -Zm176")
        else ()
  #          set_target_properties(${TestProjectName} PROPERTIES COMPILE_FLAGS "-include ${pch_file}")
           
            #todo also for MSVC??? Avoid if?
            SET_TARGET_PROPERTIES(${TestProjectName} PROPERTIES OUTPUT_NAME ${TestProjectName}.run) #rename to avoid having the same name as the folder
        endif ()

        # todo include pch files in generic test project:
#        SOURCE_GROUP("pch\\include" REGULAR_EXPRESSION ../pch/include/.*)
#        SOURCE_GROUP("pch\\src" REGULAR_EXPRESSION ../pch/src/.*)

    SOURCE_GROUP("generated" REGULAR_EXPRESSION ${TestRunnerDirectory}/.*)
    SOURCE_GROUP("src" REGULAR_EXPRESSION ${TESTDIR}/src/.*)
    SOURCE_GROUP("include" REGULAR_EXPRESSION ${TESTDIR}/include/.*)

    TARGET_LINK_LIBRARIES(${TestProjectName} test vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

    copy_msvc_project_user_file(${TestProjectName})

    # Copy icons to output folder
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Debug/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Release/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/icons)

endmacro(AddTests)

