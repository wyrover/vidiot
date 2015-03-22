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

cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

if (NOT DEFINED ENV{VIDIOT_DIR})
  message (FATAL_ERROR "VIDIOT_DIR is not defined")
endif ()

project (Vidiot)
set (VIDIOT_DIR $ENV{VIDIOT_DIR})
string (REGEX REPLACE "\\\\" "/" VIDIOT_DIR ${VIDIOT_DIR})
set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${VIDIOT_DIR})
set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${VIDIOT_DIR})
set (CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}  ${VIDIOT_DIR})
list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/build/cmake")

include (build/cmake/AddDebuggingPath.cmake)
include (build/cmake/CopyHtmlFiles.cmake)
include (build/cmake/CopyImageFiles.cmake)
include (build/cmake/EnableUnityBuild.cmake)
include (build/cmake/GeneratePrecompiledHeader.cmake)
include (build/cmake/CxxTestMacros.cmake)
include (build/cmake/TestMacros.cmake)

#### VLD ####
if (MSVC)
  if (POLICY CMP0053)
    cmake_policy(SET CMP0053 OLD) # Avoid Warning
  endif ()
  include(build/cmake/FindVLD.cmake)
else ()
  set (VLD_LIBRARIES_DIR "")
  set (VLD_INCLUDE_DIR "")
endif ()

#### FFMPEG ####
if (MSVC)
  set (ENV{FFMPEGDIR} ${PROJECT_SOURCE_DIR}/ffmpeg)
else ()
  set (ENV{PKG_CONFIG_PATH} $ENV{VIDIOT_DIR}/ffmpeg/ffmpeg_build/lib/pkgconfig)
endif ()
include (build/cmake/FindFFMPEG.cmake)
find_package (FFMPEG REQUIRED COMPONENTS SWSCALE)
if (MSVC)
    file (GLOB FFMPEG_INSTALL $ENV{FFMPEGDIR}/bin/avcodec*.dll $ENV{FFMPEGDIR}/bin/avutil*.dll $ENV{FFMPEGDIR}/bin/avformat*.dll $ENV{FFMPEGDIR}/bin/swscale*.dll $ENV{FFMPEGDIR}/bin/swresample*.dll)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /D \"__STDC_CONSTANT_MACROS\" /D \"__STDC_FORMAT_MACROS\"")
else()
  set (FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} opus mp3lame fdk-aac vpx x264 theora theoraenc theoradec vorbis vorbisenc vorbisfile va bz2)
   # todo use pkg_check_modules iso listing all libraries? -- in the .pc files these libs have already been listed!!!
endif ()
add_debugging_path ($ENV{FFMPEGDIR}/bin)

#### WXWIDGETS ####
if (MSVC)
  set (WX_USE_REL_AND_DBG TRUE)
else ()
  set (ENV{PATH} $ENV{PATH}:${VIDIOT_DIR}/wxwidgets/install/bin) # To ensure that local wx-config is found
  set (wxWidgets_USE_DEBUG ON)
  if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    set (wxWidgets_USE_DEBUG OFF)
  endif ()
  set (wxWidgets_USE_UNICODE ON)
  set (wxWidgets_USE_UNIVERSAL OFF)
  set (wxWidgets_USE_STATIC ON)
endif ()
include (build/cmake/FindwxWidgets.cmake)
find_package (wxWidgets REQUIRED core base aui adv xml html qa)

#### BOOST ####
if (MSVC)
  set (Boost_DEBUG TRUE)
  set (Boost_USE_STATIC_LIBS OFF)
  set (Boost_USE_STATIC_RUNTIME OFF)
  set (Boost_USE_MULTITHREADED ON)
  if (EXISTS "$ENV{BOOST_ROOT}/stage/lib")
    set (BOOST_LIBRARYDIR $ENV{BOOST_ROOT}/stage/lib) # Locally compiled boost
  else ()
    set (BOOST_INCLUDEDIR $ENV{BOOST_ROOT}/boost) # Prebuilt boost
    set (BOOST_LIBRARYDIR $ENV{BOOST_ROOT}/lib32-msvc-12.0)
  endif ()
else ()
  set (Boost_USE_STATIC_LIBS ON)
  set (Boost_USE_STATIC_RUNTIME ON)
endif ()
find_package (Boost 1.56 REQUIRED COMPONENTS date_time thread serialization system)
if (MSVC)
  # Find all dlls matching 'release' (== not gd) and 'threading-multi' (== mt)
  # Note: For some boost libraries, there is no dependency for building (above), but these dlls
  #       are still required (due to dependencies) when installing into C:\Program Files.
  foreach (BOOSTLIB chrono date_time thread serialization system)
    file (GLOB_RECURSE BOOST_INSTALL_NEW ${BOOST_LIBRARYDIR}/*.dll)
    foreach (DLLNAME ${BOOST_INSTALL_NEW})
      if ((${DLLNAME} MATCHES "boost_${BOOSTLIB}-.*") AND (${DLLNAME} MATCHES "-mt-"))
        if (${DLLNAME} MATCHES "-gd-")
          set (BOOST_INSTALL_DEBUG ${BOOST_INSTALL_DEBUG} ${DLLNAME})
          message ("Adding DLL for Debug installer: ${DLLNAME}")
        else ()
          set(BOOST_INSTALL ${BOOST_INSTALL} ${DLLNAME})
          message( "Adding DLL for Release installer: ${DLLNAME}" )
        endif ()
      endif ()
    endforeach ()
  endforeach ()
  add_debugging_path (${BOOST_LIBRARYDIR})
endif ()

#### PORTAUDIO ####
if (MSVC)
    set (PORTAUDIO_INCLUDE_DIR ${VIDIOT_DIR}/portaudio_trunk/include ${VIDIOT_DIR}/portaudio_trunk/src/common)
    set (PORTAUDIO_LIBRARY_DIR ${VIDIOT_DIR}/portaudio_build/Release)
    set (PORTAUDIO_LIBRARIES   portaudio_x86.dll)
    set (PORTAUDIO_INSTALL ${PORTAUDIO_LIBRARY_DIR}/portaudio_x86.dll)
    add_debugging_path (${PORTAUDIO_LIBRARY_DIR})
else ()
    set (PORTAUDIO_INCLUDE_DIR /usr/include)
    set (PORTAUDIO_LIBRARIES   portaudio asound pthread)
endif ()

#### SOUNDTOUCH ####
if (MSVC)
  set (SOUNDTOUCH_INCLUDE_DIR ${VIDIOT_DIR}/soundtouch_trunk/include)
  set (SOUNDTOUCH_LIBRARY_DIR ${VIDIOT_DIR}/soundtouch_trunk/lib)
  set (SOUNDTOUCH_LIBRARIES debug SoundTouchD.lib optimized SoundTouch.lib)
else ()
  set (SOUNDTOUCH_INCLUDE_DIR /usr/include/soundtouch)
  set (SOUNDTOUCH_LIBRARY_DIR /usr/lib/i386-linux-gnu)
  set (SOUNDTOUCH_LIBRARIES   libSoundTouch.so)
endif ()

#### FIND ALL LOCAL FILES ####
set (SOURCES_LIST)
set (HEADERS_LIST)
set (INCLUDE_LIST)

macro (findFiles dir)
  file (GLOB include ${PROJECT_SOURCE_DIR}/${dir}/include/*.h)
  file (GLOB src ${PROJECT_SOURCE_DIR}/${dir}/src/*.cpp)
  set (HEADERS_LIST ${HEADERS_LIST} ${include})
  set (SOURCES_LIST ${SOURCES_LIST} ${src})
  set (${dir}_H ${include})
  set (${dir}_CPP ${src})
  set (INCLUDE_LIST ${INCLUDE_LIST} ${PROJECT_SOURCE_DIR}/${dir}/include)
endmacro ()


set (VIDIOTPROJECTGROUPS model modelcmd modelaudio modelempty modelevent modelfile modelproject modeltransition modelvideo dialog gui guievent pch projectview projectviewcmd render timeline timelinecmd timelineevent timelinestate timelineview commands util worker)
foreach (FOLDER ${VIDIOTPROJECTGROUPS})
  findFiles (${FOLDER})
  if (NOT FOLDER STREQUAL "pch")
    enable_unity_build (${FOLDER} ${FOLDER}_CPP UNITY_CPP_FILES)
  endif ()
endforeach ()

# Below, ??*.* instead of *.* is used to avoid adding .svn files
file (GLOB BuildFiles        ${PROJECT_SOURCE_DIR}/build/??*.*)
file (GLOB CmakeFiles        ${PROJECT_SOURCE_DIR}/build/cmake/??*.*) # Ensure that these files end up in visual studio solution
list (REMOVE_ITEM BuildFiles "${PROJECT_SOURCE_DIR}/build/cxxtest-3.10.1") # Remove cxxtest from the visual studio solution

#### COMPILE, INCLUDE, AND LINK SETTINGS ####
if (MSVC)
    set (SINGLE_THREADED $ENV{VIDIOT_COMPILE_SINGLETHREADED})
    if (NOT SINGLE_THREADED)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")
    endif ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX ") # Treat warnings as errors
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
    set (CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO") # Required for avcodec
    string (REPLACE "/ZI" "/Zi" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # This ensures that in log lines not the full path is shown
    add_definitions (/D _CRT_SECURE_NO_WARNINGS)
    add_definitions (/D SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}") # Used to fix the path to the test input files (part of sources) in the code, via the preprocessor.
  include_directories (AFTER ctypes)
else ()
    list (APPEND CMAKE_CXX_FLAGS "-std=c++0x") # Add -H For checking that pch is used
    add_definitions (-DSOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}) # Used to fix the path to the test input files (part of sources) in the code, via the preprocessor.
endif ()

# CMAKE_CURRENT_BINARY_DIR is put in the front to ensure that a gcc PCH is found before the associated .h file.
include_directories (AFTER
  ${CMAKE_CURRENT_BINARY_DIR}
  ${INCLUDE_LIST}
  ${VLD_INCLUDE_DIR}
  ${FFMPEG_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
  ${PORTAUDIO_INCLUDE_DIR}
  ${SOUNDTOUCH_INCLUDE_DIR}
)

link_directories (
  ${FFMPEG_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
  ${PORTAUDIO_LIBRARY_DIR}
  ${SOUNDTOUCH_LIBRARY_DIR}
  ${VLD_LIBRARIES_DIR}
)

include (${wxWidgets_USE_FILE})

#### MAKE GROUPING FOR VISUAL STUDIO ####
if (MSVC)
  macro (groupFiles dir)
    source_group ("${dir}\\include"    REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/include/.*)
    source_group ("${dir}\\src"        REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/src/.*)
  endmacro ()

  foreach( FOLDER  ${VIDIOTPROJECTGROUPS} )
    groupFiles(${FOLDER})
  endforeach(FOLDER)
  source_group ("build" FILES ${BuildFiles} ${CmakeFiles} ${CmakeModulesFiles})
  set_source_files_properties (${BuildFiles} ${CmakeFiles} ${CmakeModulesFiles} PROPERTIES HEADER_FILE_ONLY TRUE)
endif ()

add_library (vidiot ${UNITY_CPP_FILES} ${SOURCES_LIST} ${HEADERS_LIST} ${BuildFiles} ${CmakeFiles} ${CmakeModulesFiles})

create_precompiled_header ("vidiot" "pch" "Precompiled")

# A custom target that is always built to ensure that the current revision is available in SubversionRevision.h
add_custom_target (svnrevision ALL)
add_custom_command (TARGET svnrevision COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/GetSubversionRevision.cmake)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SubversionRevision.h PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE)
add_dependencies (vidiot svnrevision)

if (POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD) # Avoid COMPILE_DEFINITIONS_* Warning
endif ()

add_subdirectory (main)
add_subdirectory (test)
add_subdirectory (install)
