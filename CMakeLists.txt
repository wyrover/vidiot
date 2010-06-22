cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

PROJECT(Vidiot)
SET(ROOT ${PROJECT_SOURCE_DIR}\\..)

SET(GENERATOR_MINGW (${CMAKE_GENERATOR} STREQUAL "MinGW Makefiles"))

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/. )

#### FFMPEG ####
SET( ENV{FFMPEGDIR} ${ROOT}\\ffmpeg_trunk )
INCLUDE(FindFFMPEG.cmake)
FIND_PACKAGE(FFMPEG)
IF(NOT FFMPEG_FOUND)
  MESSAGE("FFMPEG not found!")
  RETURN()
ENDIF(NOT FFMPEG_FOUND)
IF(NOT FFMPEG_SWSCALE_FOUND)
  MESSAGE("FFMPEG SWSCALE not found!")
  RETURN()
ENDIF(NOT FFMPEG_SWSCALE_FOUND)
SET( ENV{PATH} ENV{PATH};${FFMPEG_BINARY_DIR} )

# Add include dir for generated avconfig.h
SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} $ENV{FFMPEGDIR}/build )
# Add include for ctypes (C99 vs. MSVC)
SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} ${ROOT}/ffmpeg-ctypes )
 
message(${FFMPEG_BINARY_DIR})
message(${FFMPEG_INCLUDE_DIR})
message( $ENV{PATH} )

#### WXWIDGETS ####
SET(wxWidgets_USE_LIBS base core aui adv qa xml)
FIND_PACKAGE(wxWidgets)
IF(NOT wxWidgets_FOUND)
  MESSAGE("wxWidgets not found!")
  RETURN()
ENDIF(NOT wxWidgets_FOUND)

#### BOOST ####
SET(Boost_ADDITIONAL_VERSIONS "1.43" "1.43.0") 
IF (${GENERATOR_MINGW})
  SET( ENV{BOOST_ROOT} C:\\Boost )
  SET( ENV{BOOST_INCLUDEDIR} C:\\Boost\\include )
  SET( ENV{BOOST_LIBRARYDIR} C:\\Boost\\lib )
  SET ( Boost_COMPILER -mgw34 )
ELSE (${GENERATOR_MINGW})
  SET( ENV{BOOST_ROOT} ${ROOT}\\boost_1_43_0 )
ENDIF (${GENERATOR_MINGW})
SET(Boost_DEBUG TRUE)
#SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
Find_Package ( Boost 1.43 COMPONENTS date_time thread filesystem)
if ( NOT Boost_FOUND )
   message ( FATAL_ERROR "BOOST not found!" )
   RETURN()
ENDIF ( NOT Boost_FOUND )
message( ${Boost_LIBRARY_DIRS} )

#### DOXYGEN ####
FIND_PACKAGE(Doxygen)
IF(NOT DOXYGEN_FOUND)
  MESSAGE("DOXYGEN not found!")
  RETURN()
ENDIF(NOT DOXYGEN_FOUND)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.txt ${CMAKE_CURRENT_BINARY_DIR}/doxy.config @ONLY )
SET(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/doxy.config")
ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})

#### PORTAUDIO ####
SET(PORTAUDIO_INCLUDE_DIR ${ROOT}/portaudio_trunk/include)
SET(PORTAUDIO_LIBRARY_DIR ${ROOT}/portaudio_trunk/build/msvc/Win32/Debug)
SET(PORTAUDIO_LIBRARIES   portaudio_x86.dll)





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
FINDFILES(model_event)
FINDFILES(gui)
FINDFILES(commands)
FINDFILES(util)
FILE(GLOB Icons             ${PROJECT_SOURCE_DIR}/icons/*?.???)
FILE(GLOB Cursors           ${PROJECT_SOURCE_DIR}/cursors/*?.???)
FILE(GLOB CmakeModulesFiles ${PROJECT_SOURCE_DIR}/*.cmake)
FILE(GLOB TextFiles         ${PROJECT_SOURCE_DIR}/*.txt)

#### COMPILE, INCLUDE, AND LINK SETTINGS ####
IF (WIN32)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS)
ENDIF (WIN32)
INCLUDE_DIRECTORIES(include icons cursors ${FFMPEG_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${PORTAUDIO_INCLUDE_DIR} ${INCLUDE_LIST})
LINK_DIRECTORIES(${FFMPEG_LIBRARY_DIR} ${Boost_LIBRARY_DIRS} ${PORTAUDIO_LIBRARY_DIR})
INCLUDE(${wxWidgets_USE_FILE})
ADD_EXECUTABLE(Vidiot WIN32 ${HEADERS_LIST} ${SOURCES_LIST} ${Icons} ${Cursors} ${CmakeModulesFiles} ${TextFiles} )
TARGET_LINK_LIBRARIES(Vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES})

#### MAKE GROUPING FOR VISUAL STUDIO ####

MACRO(GROUPFILES dir)
    SOURCE_GROUP("${dir}\\include"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/include/.*)
    SOURCE_GROUP("${dir}\\src"      REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/${dir}/src/.*)
ENDMACRO(GROUPFILES)
GROUPFILES(model)
GROUPFILES(model_event)
GROUPFILES(gui)
GROUPFILES(commands)
GROUPFILES(util)
SOURCE_GROUP("icons"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/icons/*.*)
SOURCE_GROUP("cursors"  REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/cursors/*.*)
SOURCE_GROUP("cfg" FILES ${CmakeModulesFiles} ${TextFiles})
set_source_files_properties(${CmakeFiles} ${ConfigurationFiles} PROPERTIES HEADER_FILE_ONLY TRUE)



 