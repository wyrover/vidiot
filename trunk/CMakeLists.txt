cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(Vidiot)
SET(VIDIOT_DIR ${PROJECT_SOURCE_DIR}/..)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${VIDIOT_DIR} )
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${VIDIOT_DIR} )
SET(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}  ${VIDIOT_DIR} )
SET(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/build )

# Needed for Makefiles/Code::Blocks
#SET( CMAKE_BUILD_TYPE DEBUG )

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

#### FFMPEG ####
SET( ENV{FFMPEGDIR} ${VIDIOT_DIR}/ffmpeg-precompiled )
INCLUDE(build/FindFFMPEG.cmake)
FIND_PACKAGE(FFMPEG REQUIRED COMPONENTS SWSCALE)
IF (MSVC)
    FILE(GLOB FFMPEG_INSTALL $ENV{FFMPEGDIR}/bin/*.dll)
ENDIF (MSVC)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa /D \"__STDC_CONSTANT_MACROS\"")
ADD_DLL_PATH($ENV{FFMPEGDIR}/bin)

#### WXWIDGETS ####
SET( WX_USE_REL_AND_DBG TRUE )
#SET( wxWidgets_CONFIGURATION mswu ) # or mswud or mswd
FIND_PACKAGE(wxWidgets REQUIRED core base aui adv xml html qa)

#### BOOST ####
SET(Boost_ADDITIONAL_VERSIONS ENV{FOUND_BOOST_VERSION})
SET(Boost_DEBUG TRUE)
#SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost 1.45 REQUIRED COMPONENTS date_time thread filesystem serialization system)
# Find all dlls matching 'release' and 'threading-multi'
# Note: For some boost libraries, there is no dependency for building (above), but these dlls 
#       are still required (due to dependencies) when installing into C:\Program Files.
foreach(BOOSTLIB chrono date_time thread filesystem serialization system)
    FILE( GLOB_RECURSE BOOST_INSTALL_NEW $ENV{BOOST_ROOT}/bin.v2/libs/${BOOSTLIB}/*.dll )
    foreach( DLLNAME ${BOOST_INSTALL_NEW} )
        if((${DLLNAME} MATCHES "release") AND (${DLLNAME} MATCHES "threading-multi"))
            SET( BOOST_INSTALL ${BOOST_INSTALL} ${DLLNAME} )
        endif((${DLLNAME} MATCHES "release") AND (${DLLNAME} MATCHES "threading-multi"))
    endforeach(DLLNAME)
endforeach(BOOSTLIB)
ADD_DLL_PATH($ENV{BOOST_ROOT}/stage/lib)

#### PORTAUDIO ####
SET(PORTAUDIO_INCLUDE_DIR ${VIDIOT_DIR}/portaudio_trunk/include ${VIDIOT_DIR}/portaudio_trunk/src/common)
SET(PORTAUDIO_LIBRARY_DIR ${VIDIOT_DIR}/portaudio_trunk/build/msvc/Win32/Release)
SET(PORTAUDIO_LIBRARIES   portaudio_x86.dll)
SET(PORTAUDIO_INSTALL ${PORTAUDIO_LIBRARY_DIR}/portaudio_x86.dll)
ADD_DLL_PATH(${PORTAUDIO_LIBRARY_DIR})

#### SOUNDTOUCH ####
SET( SOUNDTOUCH_ROOT        ${VIDIOT_DIR}/soundtouch_trunk )
SET( SOUNDTOUCH_INCLUDE_DIR ${SOUNDTOUCH_ROOT}/include )
SET( SOUNDTOUCH_LIBRARY_DIR ${SOUNDTOUCH_ROOT}/lib )
IF (MSVC)
  SET( SOUNDTOUCH_LIBRARIES debug SoundTouchDllD.dll debug SoundTouchD.lib optimized SoundTouchDll.dll optimized SoundTouch.lib )
  FILE( GLOB SOUNDTOUCH_INSTALL ${SOUNDTOUCH_LIBRARY_DIR}/*.dll )
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

SET( VIDIOTPROJECTGROUPS model modelcmd modelaudio modelempty modelevent modelproject modelvideo gui guievent pch projectview projectviewcmd render timeline timelinecmd timelineevent timelinestate timelineview commands util )
FOREACH( FOLDER  ${VIDIOTPROJECTGROUPS} )
    FINDFILES(${FOLDER})
ENDFOREACH(FOLDER)
# Below, ??*.* instead of *.* is used to avoid adding .svn files
FILE(GLOB Icons             ${PROJECT_SOURCE_DIR}/icons/*?.???)
FILE(GLOB Cursors           ${PROJECT_SOURCE_DIR}/cursors/*?.???)
FILE(GLOB BuildFiles        ${PROJECT_SOURCE_DIR}/build/??*.*)
FILE(GLOB_RECURSE HtmlFiles ${PROJECT_SOURCE_DIR}/html/*.*)

#### COMPILE, INCLUDE, AND LINK SETTINGS ####
IF (MSVC)
    SET(SINGLE_THREADED $ENV{VIDIOT_COMPILE_SINGLETHREADED} )
    IF (NOT SINGLE_THREADED)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")
    ENDIF (NOT SINGLE_THREADED)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
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
SOURCE_GROUP("icons"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/icons/*.*)
SOURCE_GROUP("cursors"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/cursors/*.*)
SOURCE_GROUP("build" FILES ${BuildFiles} ${CmakeModulesFiles})
SOURCE_GROUP("html\\about" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/html/about/.*)
SOURCE_GROUP("html\\help" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/html/help/.*)
SET_SOURCE_FILES_PROPERTIES(${CmakeFiles} ${BuildFiles} PROPERTIES HEADER_FILE_ONLY TRUE)

ADD_LIBRARY(vidiot ${SOURCES_LIST} ${HEADERS_LIST} ${Icons} ${Cursors} ${HtmlFiles} ${BuildFiles})

ADD_SUBDIRECTORY(main)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(install)

if (MSVC)
   set_target_properties(vidiot PROPERTIES COMPILE_FLAGS "/YuPrecompiled.h")
   set_source_files_properties(pch/src/Precompiled.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiled.h")
endif(MSVC)