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

set(TESTDIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(../test/CxxTestMacros.inc)

INCLUDE_DIRECTORIES(AFTER
    ../pch/include
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../test/include
)

macro(AddTests TestProjectName)
    file(GLOB TestHeaders ${TESTDIR}/*/*.h)
    file(GLOB TestSources ${TESTDIR}/*/*.cpp)

    if (${TestProjectName} STREQUAL "testauto")
        FILE(GLOB pch_include ${TESTDIR}/../pch/include/*.h)
        FILE(GLOB pch_src ${TESTDIR}/../pch//src/*.cpp)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FIPrecompiledTest.h ")
    endif (${TestProjectName} STREQUAL "testauto")

    ENABLE_UNITY_BUILD(${TestProjectName} TestSources TEST_AUTO_UNITY_CPP_FILES)

    AddCxxTest(${TestProjectName} ${pch_include} ${TEST_AUTO_UNITY_CPP_FILES} ${pch_src} ${TestHeaders} ${TestSources})

    if (${TestProjectName} STREQUAL "testauto")
        if (MSVC)
            set_target_properties(${TestProjectName} PROPERTIES COMPILE_FLAGS "/YuPrecompiledTest.h -Zm176")
            set_source_files_properties(${TESTDIR}/../pch/src/PrecompiledTest.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiledTest.h")
        endif(MSVC)
        SOURCE_GROUP("pch\\include" REGULAR_EXPRESSION ../pch/include/.*)
        SOURCE_GROUP("pch\\src" REGULAR_EXPRESSION ../pch/src/.*)
    endif (${TestProjectName} STREQUAL "testauto")

    SOURCE_GROUP("generated" REGULAR_EXPRESSION ${TestRunnerDirectory}/.*)
    SOURCE_GROUP("src" REGULAR_EXPRESSION ${TESTDIR}/src/.*)
    SOURCE_GROUP("include" REGULAR_EXPRESSION ${TESTDIR}/include/.*)

    TARGET_LINK_LIBRARIES(${TestProjectName} test vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

    COPY_MSVC_PROJECT_USER_FILE(${TestProjectName})

    # Copy icons to output folder
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Debug/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Release/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/icons)

endmacro(AddTests)

