cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(Vidiot)
SET(ROOT ${PROJECT_SOURCE_DIR}\\..)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${ROOT}/build )
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${ROOT}/build )
SET(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}  ${ROOT}/build )
SET(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${CMAKE_SOURCE_DIR}/. )


# Needed for Makefiles/Code::Blocks
#SET( CMAKE_BUILD_TYPE DEBUG )

 
#### FFMPEG ####
SET( ENV{FFMPEGDIR} ${ROOT}\\ffmpeg-precompiled )
#_trunk )
INCLUDE(FindFFMPEG.cmake)
FIND_PACKAGE(FFMPEG REQUIRED COMPONENTS SWSCALE)
SET( ENV{PATH} ENV{PATH};${FFMPEG_BINARY_DIR} )
# Add include dir for generated avconfig.h
SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} $ENV{FFMPEGDIR}/build )
# Add include for ctypes (C99 vs. MSVC)
IF (MSVC)
    SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} ${ROOT}/ffmpeg-ctypes )
ENDIF (MSVC)

#### WXWIDGETS ####
#GCC find_package(wxWidgets REQUIRED core base aui adv xml html qa)
SET(wxWidgets_USE_LIBS base core aui adv qa xml html)
FIND_PACKAGE(wxWidgets REQUIRED)

#### BOOST ####
SET(Boost_ADDITIONAL_VERSIONS "1.43" "1.43.0")
  SET( ENV{BOOST_ROOT} ${ROOT}\\boost_1_43_0 )
SET(Boost_DEBUG TRUE)
#SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
Find_Package ( Boost 1.43 REQUIRED COMPONENTS date_time thread filesystem)
#GCC serialization system)

#### DOXYGEN ####
FIND_PACKAGE(Doxygen REQUIRED)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.txt ${CMAKE_CURRENT_BINARY_DIR}/doxy.config @ONLY )
SET(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/doxy.config")
ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})

#### PORTAUDIO ####
SET(PORTAUDIO_INCLUDE_DIR ${ROOT}/portaudio_trunk/include)
SET(PORTAUDIO_LIBRARY_DIR ${ROOT}/portaudio_trunk/build/msvc/Win32/Debug)
SET(PORTAUDIO_LIBRARIES   portaudio_x86.dll)

#### SOUNDTOUCH ####
SET( SOUNDTOUCH_ROOT        ${ROOT}/soundtouch_trunk )
SET( SOUNDTOUCH_INCLUDE_DIR ${SOUNDTOUCH_ROOT}/include )
SET( SOUNDTOUCH_LIBRARY_DIR ${SOUNDTOUCH_ROOT}/lib )
IF (MSVC)
  SET( SOUNDTOUCH_LIBRARIES   SoundTouchDllD.dll SoundTouchD.lib )
ENDIF (MSVC)
#IF (CMAKE_COMPILER_IS_GNUCC)
#  SET( SOUNDTOUCH_LIBRARIES  libSoundTouch.a )
#ENDIF (CMAKE_COMPILER_IS_GNUCC )
  #gcc_s )

#SoundTouchDllD.lib SoundTouchD.lib SoundTouchDll.lib SoundTouch.lib
# ${SOUNDTOUCH_ROOT}/source/SoundTouch/Release/SoundTouch.lib )
#D:\Vidiot\soundtouch_trunk\source\SoundTouchDLL\Release

#### FIND ALL LOCAL FILES ####
SET(SOURCES_LIST)
SET(HEADERS_LIST)
SET(INCLUDE_LIST)

MACRO(FINDFILES dir)
    FILE(GLOB headers ${PROJECT_SOURCE_DIR}/${dir}/include/*.h)
    FILE(GLOB sources ${PROJECT_SOURCE_DIR}/${dir}/src/*.cpp)
    SET(HEADERS_LIST ${HEADERS_LIST} ${headers})
    SET(SOURCES_LIST ${SOURCES_LIST} ${sources})
    SET(${dir}_H ${headers})
    SET(${dir}_CPP ${sources})
    SET(INCLUDE_LIST ${INCLUDE_LIST} ${PROJECT_SOURCE_DIR}/${dir}/include)
ENDMACRO(FINDFILES)
FINDFILES(model)
FINDFILES(gui)
FINDFILES(timeline)
FINDFILES(commands)
FINDFILES(util)
FILE(GLOB Icons             ${PROJECT_SOURCE_DIR}/icons/*?.???)
FILE(GLOB Cursors           ${PROJECT_SOURCE_DIR}/cursors/*?.???)
FILE(GLOB CmakeModulesFiles ${PROJECT_SOURCE_DIR}/*.cmake)
FILE(GLOB TextFiles         ${PROJECT_SOURCE_DIR}/*.txt)
FILE(GLOB HelpFiles         ${PROJECT_SOURCE_DIR}/help/*.*)

#### COMPILE, INCLUDE, AND LINK SETTINGS ####
IF (MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS)
ENDIF (MSVC)

#foreach(flag_var
#        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
#        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#   if(${flag_var} MATCHES "/MD")
#      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#   endif(${flag_var} MATCHES "/MD")
#endforeach(flag_var)


INCLUDE_DIRECTORIES(include
    icons
    cursors
    ${FFMPEG_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIR}
    ${INCLUDE_LIST}
    ${SOUNDTOUCH_INCLUDE_DIR}
)
LINK_DIRECTORIES(
    ${FFMPEG_LIBRARY_DIR}
    ${Boost_LIBRARY_DIRS}
    ${PORTAUDIO_LIBRARY_DIR}
    ${SOUNDTOUCH_LIBRARY_DIR}
)
INCLUDE(${wxWidgets_USE_FILE})
ADD_EXECUTABLE(Vidiot WIN32 ${HEADERS_LIST} ${SOURCES_LIST} ${Icons} ${Cursors} ${CmakeModulesFiles} ${TextFiles} ${HelpFiles} HelpFiles)
TARGET_LINK_LIBRARIES(Vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

#### MAKE GROUPING FOR VISUAL STUDIO ####

MACRO(GROUPFILES dir)
    SOURCE_GROUP("${dir}\\include"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/include/.*)
    SOURCE_GROUP("${dir}\\src"      REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/src/.*)
ENDMACRO(GROUPFILES)
GROUPFILES(model)
GROUPFILES(gui)
GROUPFILES(timeline)
GROUPFILES(commands)
GROUPFILES(util)
SOURCE_GROUP("icons"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/icons/*.*)
SOURCE_GROUP("cursors"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/cursors/*.*)
SOURCE_GROUP("cfg" FILES ${CmakeModulesFiles} ${TextFiles})
SOURCE_GROUP("help" FILES ${HelpFiles})
set_source_files_properties(${CmakeFiles} ${ConfigurationFiles} PROPERTIES HEADER_FILE_ONLY TRUE)

#### HELP FILES ####
add_custom_command(OUTPUT HelpFiles
COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/help ${PROJECT_BINARY_DIR}/help)
