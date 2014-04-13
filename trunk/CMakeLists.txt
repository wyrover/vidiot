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

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(Vidiot)
SET(VIDIOT_DIR ${PROJECT_SOURCE_DIR}/..)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${VIDIOT_DIR} )
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${VIDIOT_DIR} )
SET(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}  ${VIDIOT_DIR} )
SET(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/build )


# Fill this variable with extra dll paths required for running the application
# in a debugging context. The variable is used to set MSVC debugging paths.
# This avoids having to change the system path for running with dlls from E.g.
# ffmpeg, boost, and portaudio in the debugger.
SET(MSVC_DLL_PATH "")
SET(MSVC_USER_TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/build/template.vcxproj.user)
MACRO(ADD_DLL_PATH dllpath)
    IF (MSVC)
        message( "Adding debugging path: " ${dllpath} ) 
        SET(MSVC_DLL_PATH ${MSVC_DLL_PATH};${dllpath})
    ENDIF (MSVC)
ENDMACRO(ADD_DLL_PATH)
MACRO(COPY_MSVC_PROJECT_USER_FILE projectname)
    CONFIGURE_FILE(${MSVC_USER_TEMPLATE_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${projectname}.vcxproj.user @ONLY)
ENDMACRO(COPY_MSVC_PROJECT_USER_FILE)

#### VLD ####
IF (MSVC)
    INCLUDE(build/FindVLD.cmake)
    IF (NOT VLD_FOUND)
        message( FATAL_ERROR "VLD NOT FOUND." )
    ENDIF (NOT VLD_FOUND)
ENDIF (MSVC)

#### FFMPEG ####
SET( ENV{FFMPEGDIR} ${PROJECT_SOURCE_DIR}/ffmpeg )
INCLUDE(build/FindFFMPEG.cmake)
FIND_PACKAGE(FFMPEG REQUIRED COMPONENTS SWSCALE )
IF (MSVC)
    FILE(GLOB FFMPEG_INSTALL $ENV{FFMPEGDIR}/bin/avcodec*.dll $ENV{FFMPEGDIR}/bin/avutil*.dll $ENV{FFMPEGDIR}/bin/avformat*.dll $ENV{FFMPEGDIR}/bin/swscale*.dll $ENV{FFMPEGDIR}/bin/swresample*.dll)
ENDIF (MSVC)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /D \"__STDC_CONSTANT_MACROS\" /D \"__STDC_FORMAT_MACROS\"")
ADD_DLL_PATH($ENV{FFMPEGDIR}/bin)

#### WXWIDGETS ####
SET( WX_USE_REL_AND_DBG TRUE )
FIND_PACKAGE(wxWidgets REQUIRED core base aui adv xml html qa)

#### BOOST ####
SET(Boost_ADDITIONAL_VERSIONS ENV{FOUND_BOOST_VERSION})
SET(Boost_DEBUG TRUE)
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_STATIC_RUNTIME OFF)
SET(Boost_USE_MULTITHREADED ON)
IF(EXISTS "$ENV{BOOST_ROOT}/stage/lib")
  # Locally compiled boost
  SET(BOOST_LIBRARYDIR $ENV{BOOST_ROOT}/stage/lib)
ELSE()
  # Prebuilt boost
  SET(BOOST_INCLUDEDIR $ENV{BOOST_ROOT}/boost)
  SET(BOOST_LIBRARYDIR $ENV{BOOST_ROOT}/lib32-msvc-11.0)
ENDIF()
FIND_PACKAGE(Boost 1.55 REQUIRED COMPONENTS date_time thread serialization system)
# Find all dlls matching 'release' (== not gd) and 'threading-multi' (== mt)
# Note: For some boost libraries, there is no dependency for building (above), but these dlls 
#       are still required (due to dependencies) when installing into C:\Program Files.
foreach(BOOSTLIB chrono date_time thread serialization system)
    FILE( GLOB_RECURSE BOOST_INSTALL_NEW ${BOOST_LIBRARYDIR}/*.dll )
    foreach( DLLNAME ${BOOST_INSTALL_NEW} )
        if((${DLLNAME} MATCHES "boost_${BOOSTLIB}-.*") AND (${DLLNAME} MATCHES "-mt-"))
            if( ${DLLNAME} MATCHES "-gd-" ) 
                SET( BOOST_INSTALL_DEBUG ${BOOST_INSTALL_DEBUG} ${DLLNAME} )
                message( "Adding DLL for Debug installer: ${DLLNAME}" )
            else()
                SET( BOOST_INSTALL ${BOOST_INSTALL} ${DLLNAME} )
                message( "Adding DLL for Release installer: ${DLLNAME}" )
            endif()
        endif()
    endforeach(DLLNAME)
endforeach(BOOSTLIB)
ADD_DLL_PATH(${BOOST_LIBRARYDIR})

#### PORTAUDIO ####
SET(PORTAUDIO_INCLUDE_DIR ${VIDIOT_DIR}/portaudio_trunk/include ${VIDIOT_DIR}/portaudio_trunk/src/common)
SET(PORTAUDIO_LIBRARY_DIR ${VIDIOT_DIR}/portaudio_build/Release)
SET(PORTAUDIO_LIBRARIES   portaudio_x86.dll)
SET(PORTAUDIO_INSTALL ${PORTAUDIO_LIBRARY_DIR}/portaudio_x86.dll)
ADD_DLL_PATH(${PORTAUDIO_LIBRARY_DIR})

#### SOUNDTOUCH ####
SET( SOUNDTOUCH_ROOT        ${VIDIOT_DIR}/soundtouch_trunk )
SET( SOUNDTOUCH_INCLUDE_DIR ${SOUNDTOUCH_ROOT}/include )
SET( SOUNDTOUCH_LIBRARY_DIR ${SOUNDTOUCH_ROOT}/lib )
IF (MSVC)
  SET( SOUNDTOUCH_LIBRARIES debug SoundTouchDllD.dll debug SoundTouchD.lib optimized SoundTouchDll.dll optimized SoundTouch.lib )
  FILE( GLOB SOUNDTOUCH_INSTALL ${SOUNDTOUCH_LIBRARY_DIR}/SoundTouch.dll )
ENDIF (MSVC)
IF (CMAKE_COMPILER_IS_GNUCC)
  SET( SOUNDTOUCH_LIBRARIES  libSoundTouch.a )
ENDIF (CMAKE_COMPILER_IS_GNUCC )

#### FIND ALL LOCAL FILES ####
SET(SOURCES_LIST)
SET(HEADERS_LIST)
SET(INCLUDE_LIST)

MACRO(FINDFILES dir)
    FILE(GLOB include ${PROJECT_SOURCE_DIR}/${dir}/include/*.h)
    FILE(GLOB src ${PROJECT_SOURCE_DIR}/${dir}/src/*.cpp)
    SET(HEADERS_LIST ${HEADERS_LIST} ${include})
    SET(SOURCES_LIST ${SOURCES_LIST} ${src})
    SET(${dir}_H ${include})
    SET(${dir}_CPP ${src})
    SET(INCLUDE_LIST ${INCLUDE_LIST} ${PROJECT_SOURCE_DIR}/${dir}/include)
ENDMACRO(FINDFILES)

# See
#    http://buffered.io/posts/the-magic-of-unity-builds/ ("The Magic of Unity Builds")
#    http://cheind.wordpress.com/2009/12/10/reducing-compilation-time-unity-builds/ ("Reducing Compilation Time: Unity Builds")
# On why this helps speed up compilation a lot (even when using a SSD).
# At the time I introduced this, build time was improved from 10:00 to 3:10 on my pc.
FUNCTION(ENABLE_UNITY_BUILD FOLDER FILES OUTPUT_FILES)
    SET(UNITY_CPP ${CMAKE_CURRENT_BINARY_DIR}/unity_${FOLDER}.cpp)
    SET_SOURCE_FILES_PROPERTIES(${${FILES}} PROPERTIES HEADER_FILE_ONLY true)
    FILE(WRITE ${UNITY_CPP} "// Unity Build generated by CMake\n")
    FOREACH(SOURCE_FILE ${${FILES}} )
        FILE( APPEND ${UNITY_CPP} "#include <${SOURCE_FILE}>\n")
    ENDFOREACH(SOURCE_FILE)
    SET(OUTPUT_FILES_LOCAL ${${OUTPUT_FILES}} ${UNITY_CPP})
    SET(${OUTPUT_FILES} ${OUTPUT_FILES_LOCAL} PARENT_SCOPE) # Copy to scope of calling code
    SOURCE_GROUP("unity" FILES ${OUTPUT_FILES_LOCAL} )
ENDFUNCTION(ENABLE_UNITY_BUILD)

SET( VIDIOTPROJECTGROUPS model modelcmd modelaudio modelempty modelevent modelproject modelvideo dialog gui guievent pch projectview projectviewcmd render timeline timelinecmd timelineevent timelinestate timelineview commands util worker)
FOREACH( FOLDER  ${VIDIOTPROJECTGROUPS} )
    FINDFILES(${FOLDER})
    IF ( NOT FOLDER STREQUAL "pch" )
        ENABLE_UNITY_BUILD(${FOLDER} ${FOLDER}_CPP UNITY_CPP_FILES)
    ENDIF()
ENDFOREACH(FOLDER)
# Below, ??*.* instead of *.* is used to avoid adding .svn files
FILE(GLOB Cursors           ${PROJECT_SOURCE_DIR}/cursors/*?.???)
FILE(GLOB BuildFiles        ${PROJECT_SOURCE_DIR}/build/??*.*)
list(REMOVE_ITEM BuildFiles "${PROJECT_SOURCE_DIR}/build/cxxtest-3.10.1") # Remove cxxtest from the visual studio solution



#### COMPILE, INCLUDE, AND LINK SETTINGS ####
IF (MSVC)
    SET(SINGLE_THREADED $ENV{VIDIOT_COMPILE_SINGLETHREADED} )
    IF (NOT SINGLE_THREADED)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")
    ENDIF (NOT SINGLE_THREADED)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX ") # Treat warnings as errors
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
    SET(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO") # Required for avcodec
    # Force including Precompiled.h for all cpp files
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FIPrecompiled.h ")
    string(REPLACE "/ZI" "/Zi" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # This ensures that in log lines not the full path is shown
    ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS (/D SOURCE_ROOT="\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"") # Used to fix the path to the input files (part of sources) in the code, via the preprocessor.
ENDIF (MSVC)

INCLUDE_DIRECTORIES(AFTER
    icons
    cursors
    ctypes
    ${VLD_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIR}
    ${INCLUDE_LIST}
    ${SOUNDTOUCH_INCLUDE_DIR}
    test/test/include
    test/pch/include
)

LINK_DIRECTORIES(
    ${FFMPEG_LIBRARY_DIR}
    ${Boost_LIBRARY_DIRS}
    ${PORTAUDIO_LIBRARY_DIR}
    ${SOUNDTOUCH_LIBRARY_DIR}
    ${VLD_LIBRARIES_DIR}
)

INCLUDE(${wxWidgets_USE_FILE})

#### MAKE GROUPING FOR VISUAL STUDIO ####
MACRO(GROUPFILES dir)
    SOURCE_GROUP("${dir}\\include"    REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/include/.*)
    SOURCE_GROUP("${dir}\\src"        REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/src/.*)
ENDMACRO(GROUPFILES)

FOREACH( FOLDER  ${VIDIOTPROJECTGROUPS} )
    GROUPFILES(${FOLDER})
ENDFOREACH(FOLDER)
#SOURCE_GROUP("icons"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/icons/*.*)
SOURCE_GROUP("cursors"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/cursors/*.*)
SOURCE_GROUP("build" FILES ${BuildFiles} ${CmakeModulesFiles})
SET_SOURCE_FILES_PROPERTIES(${CmakeFiles} ${BuildFiles} PROPERTIES HEADER_FILE_ONLY TRUE)


ADD_LIBRARY(vidiot ${UNITY_CPP_FILES} ${SOURCES_LIST} ${HEADERS_LIST} ${Icons} ${Cursors} ${BuildFiles})

# A custom target that is always built to ensure that the current revision is available in SubversionRevision.h
add_custom_target(svnrevision ALL)
add_custom_command(TARGET svnrevision COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/build/GetSubversionRevision.cmake)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SubversionRevision.h PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE) 
add_dependencies(vidiot svnrevision)

ADD_SUBDIRECTORY(main)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(install)
ADD_SUBDIRECTORY(html) # Must be after the other dirs, since it uses the other targets
ADD_SUBDIRECTORY(icons) # Must be after the other dirs, since it uses the other targets

if (MSVC)
   set_target_properties(vidiot PROPERTIES COMPILE_FLAGS "/YuPrecompiled.h -Zm140")
   set_source_files_properties(pch/src/Precompiled.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiled.h")
endif(MSVC)