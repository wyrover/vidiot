cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(Vidiot)
SET(VIDIOT_DIR ${PROJECT_SOURCE_DIR}\\..)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${VIDIOT_DIR} )
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${VIDIOT_DIR} )
SET(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}  ${VIDIOT_DIR} )
SET(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/build )

# Needed for Makefiles/Code::Blocks
#SET( CMAKE_BUILD_TYPE DEBUG )

#### FFMPEG ####
#SET( ENV{FFMPEGDIR} ${VIDIOT_DIR}/ffmpeg-precompiled )
SET( ENV{FFMPEGDIR} ${VIDIOT_DIR}/ffmpeg-r26182 )
INCLUDE(build/FindFFMPEG.cmake)
FIND_PACKAGE(FFMPEG REQUIRED COMPONENTS SWSCALE)
# Add include dir for generated avconfig.h
SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} $ENV{FFMPEGDIR}/build )
# Add include for ctypes (C99 vs. MSVC)
IF (MSVC)
    SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} ${VIDIOT_DIR}/ffmpeg-ctypes )
ENDIF (MSVC)

#### WXWIDGETS ####
SET( WX_USE_REL_AND_DBG TRUE )
#SET( wxWidgets_CONFIGURATION mswu ) # or mswud or mswd
FIND_PACKAGE(wxWidgets REQUIRED core base aui adv xml html qa)

#### BOOST ####
SET(Boost_ADDITIONAL_VERSIONS "1.43" "1.43.0")
SET( ENV{BOOST_ROOT} ${VIDIOT_DIR}/boost_1_43_0 )
SET(Boost_DEBUG TRUE)
#SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost 1.43 REQUIRED COMPONENTS date_time thread filesystem serialization system)

#### DOXYGEN ####
FIND_PACKAGE(Doxygen REQUIRED)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/build/doxyfile.txt ${CMAKE_CURRENT_BINARY_DIR}/doxy.config @ONLY )
SET(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/doxy.config")
ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})

#### PORTAUDIO ####
SET(PORTAUDIO_INCLUDE_DIR ${VIDIOT_DIR}/portaudio_trunk/include)
SET(PORTAUDIO_LIBRARY_DIR ${VIDIOT_DIR}/portaudio_trunk/build/msvc/Win32/Release)
SET(PORTAUDIO_LIBRARIES   portaudio_x86.dll)

#### SOUNDTOUCH ####
SET( SOUNDTOUCH_ROOT        ${VIDIOT_DIR}/soundtouch_trunk )
SET( SOUNDTOUCH_INCLUDE_DIR ${SOUNDTOUCH_ROOT}/include )
SET( SOUNDTOUCH_LIBRARY_DIR ${SOUNDTOUCH_ROOT}/lib )
IF (MSVC)
  SET( SOUNDTOUCH_LIBRARIES_DEBUG   SoundTouchDllD.dll SoundTouchD.lib )
  SET( SOUNDTOUCH_LIBRARIES debug SoundTouchDllD.dll SoundTouchD.lib optimized SoundTouchDll.dll SoundTouch.lib )
ENDIF (MSVC)
IF (CMAKE_COMPILER_IS_GNUCC)
  SET( SOUNDTOUCH_LIBRARIES  libSoundTouch.a )
ENDIF (CMAKE_COMPILER_IS_GNUCC )
  #gcc_s )

#SoundTouchDllD.lib SoundTouchD.lib SoundTouchDll.lib SoundTouch.lib
# ${SOUNDTOUCH_ROOT}/source/SoundTouch/Release/SoundTouch.lib )
#D:\Vidiot\soundtouch_trunk\source\SoundTouchDLL\Release

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
FINDFILES(model)
FINDFILES(gui)
FINDFILES(projectview)
FINDFILES(timeline)
FINDFILES(timelinecmd)
FINDFILES(timelinestate)
FINDFILES(timelineview)
FINDFILES(commands)
FINDFILES(util)
# Below, ??*.* instead of *.* is used to avoid adding .svn files
FILE(GLOB Icons             ${PROJECT_SOURCE_DIR}/icons/*?.???)
FILE(GLOB Cursors           ${PROJECT_SOURCE_DIR}/cursors/*?.???)
FILE(GLOB BuildFiles        ${PROJECT_SOURCE_DIR}/build/??*.*)
FILE(GLOB HelpFiles         ${PROJECT_SOURCE_DIR}/help/??*.*)

#### COMPILE, INCLUDE, AND LINK SETTINGS ####
IF (MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS)
ENDIF (MSVC)

INCLUDE_DIRECTORIES(AFTER
    include
    icons
    cursors
    ${FFMPEG_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIR}
    ${INCLUDE_LIST}
    ${SOUNDTOUCH_INCLUDE_DIR}
)
#get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
#message("inc_dirs = ${inc_dirs}")

LINK_DIRECTORIES(
    ${FFMPEG_LIBRARY_DIR}
    ${Boost_LIBRARY_DIRS}
    ${PORTAUDIO_LIBRARY_DIR}
    ${SOUNDTOUCH_LIBRARY_DIR}
)
INCLUDE(${wxWidgets_USE_FILE})
ADD_EXECUTABLE(Vidiot WIN32 ${HEADERS_LIST} ${SOURCES_LIST} ${Icons} ${Cursors} ${BuildFiles} ${HelpFiles}) # Add HelpFiles Here to make Vidiot dependent on copying the helpfiles
TARGET_LINK_LIBRARIES(Vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})


#### MAKE GROUPING FOR VISUAL STUDIO ####
MACRO(GROUPFILES dir)
    SOURCE_GROUP("${dir}\\include"    REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/include/.*)
    SOURCE_GROUP("${dir}\\src"        REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/src/.*)
ENDMACRO(GROUPFILES)
GROUPFILES(model)
GROUPFILES(gui)
GROUPFILES(projectview)
GROUPFILES(timeline)
GROUPFILES(timelinecmd)
GROUPFILES(timelinestate)
GROUPFILES(timelineview)
GROUPFILES(commands)
GROUPFILES(util)
SOURCE_GROUP("icons"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/icons/*.*)
SOURCE_GROUP("cursors"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/cursors/*.*)
SOURCE_GROUP("build" FILES CMakeLists.txt ${BuildFiles} ${CmakeModulesFiles})
SOURCE_GROUP("help" FILES ${HelpFiles})
SET_SOURCE_FILES_PROPERTIES(${CmakeFiles} ${ConfigurationFiles} ${BuildFiles} PROPERTIES HEADER_FILE_ONLY TRUE)

#### HELP FILES ####
#add_custom_command(OUTPUT HelpFiles COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/help ${PROJECT_BINARY_DIR}/help)
