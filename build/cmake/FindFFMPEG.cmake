# - Try to find FFMPEG
# Once done this will define
#
#  FFMPEG_FOUND            - system has FFMPEG
#  FFMPEG_INCLUDE_DIR      - the include directories
#  FFMPEG_LIBRARY_DIR      - the directory containing the libraries
#  FFMPEG_BINARY_DIR       - the directory containing the dlls
#  FFMPEG_LIBRARIES        - link these to use FFMPEG
#  FFMPEG_SWSCALE_FOUND    - FFMPEG also has SWSCALE
#  FFMPEG_SWRESAMPLE_FOUND - FFMPEG also has SWRESAMPLE

    SET( FFMPEG_HEADERS avformat.h avcodec.h avutil.h avdevice.h swscale.h swresample.h)
    SET( FFMPEG_PATH_SUFFIXES libavformat libavcodec libavutil libavdevice libswscale libswresample)
    SET( FFMPEG_SWS_HEADERS swscale.h )
    SET( FFMPEG_SWS_PATH_SUFFIXES libswscale )
    if( WIN32 )
       SET( FFMPEG_LIBRARIES avformat.lib avcodec.lib avutil.lib avdevice.lib swresample.lib swscale.lib)
       SET( FFMPEG_SWS_LIBRARIES swscale.lib )
       SET( FFMPEG_LIBRARY_DIR $ENV{FFMPEGDIR}\\lib )
       SET( FFMPEG_BINARY_DIR $ENV{FFMPEGDIR}\\bin )
       SET( FFMPEG_INCLUDE_PATHS $ENV{FFMPEGDIR} $ENV{FFMPEGDIR}\\include )

       # check to see if we can find swscale
       SET( TMP_ TMP-NOTFOUND )
       FIND_PATH( TMP_ ${FFMPEG_SWS_LIBRARIES}
                  PATHS ${FFMPEG_LIBRARY_DIR} )
       IF ( TMP_ )
          SET( SWSCALE_FOUND TRUE )
       ENDIF( TMP_ )
    else( WIN32 )
       SET( FFMPEG_LIBRARIES avformat avcodec avutil avdevice swscale swresample)
       SET( FFMPEG_SWS_LIBRARIES swscale )
       INCLUDE(FindPkgConfig)
       if ( PKG_CONFIG_FOUND )
          pkg_check_modules( AVFORMAT libavformat )
          pkg_check_modules( AVCODEC libavcodec )
          pkg_check_modules( AVUTIL libavutil )
          pkg_check_modules( AVDEVICE libavdevice )
          pkg_check_modules( SWSCALE libswscale )
          pkg_check_modules( SWRESAMPLE libswresample )
       endif ( PKG_CONFIG_FOUND )

       SET( FFMPEG_LIBRARY_DIR   ${AVFORMAT_LIBRARY_DIRS}
                                 ${AVCODEC_LIBRARY_DIRS}
                                 ${AVUTIL_LIBRARY_DIRS}
                                 ${AVDEVICE_LIBRARY_DIRS} )
    endif( WIN32 )

    # add in swscale if found
    IF ( SWSCALE_FOUND )
       SET( FFMPEG_LIBRARY_DIR   ${FFMPEG_LIBRARY_DIR}
                                 ${SWSCALE_LIBRARY_DIRS} )
       SET( FFMPEG_INCLUDE_PATHS ${FFMPEG_INCLUDE_PATHS}
                                 ${SWSCALE_INCLUDE_DIRS} )
       SET( FFMPEG_HEADERS       ${FFMPEG_HEADERS}
                                 ${FFMPEG_SWS_HEADERS} )
       SET( FFMPEG_PATH_SUFFIXES ${FFMPEG_PATH_SUFFIXES}
                                 ${FFMPEG_SWS_PATH_SUFFIXES} )
       SET( FFMPEG_LIBRARIES     ${FFMPEG_LIBRARIES}
                                 ${FFMPEG_SWS_LIBRARIES} )
    ENDIF ( SWSCALE_FOUND )

    # find includes
    SET( INC_SUCCESS 0 )
    SET( TMP_ TMP-NOTFOUND )
    SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_PATHS} )

    # clear out duplicates
    LIST( REMOVE_DUPLICATES FFMPEG_INCLUDE_DIR )
    LIST( REMOVE_DUPLICATES FFMPEG_LIBRARY_DIR )

    # find the full paths of the libraries
    SET( TMP_ TMP-NOTFOUND )
    IF ( NOT WIN32 )
       FOREACH( LIB_ ${FFMPEG_LIBRARIES} )
          FIND_LIBRARY( TMP_ NAMES ${LIB_} PATHS ${FFMPEG_LIBRARY_DIR} )
          IF ( TMP_ )
             SET( FFMPEG_LIBRARIES_FULL ${FFMPEG_LIBRARIES_FULL} ${TMP_} )
          ENDIF ( TMP_ )
          SET( TMP_ TMP-NOTFOUND )
       ENDFOREACH( LIB_ )
       SET ( FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES_FULL} )
    ENDIF( NOT WIN32 )

    LIST( LENGTH FFMPEG_HEADERS LIST_SIZE_ )

    SET( FFMPEG_FOUND FALSE )
    SET( FFMPEG_SWSCALE_FOUND FALSE )
    IF ( ${INC_SUCCESS} EQUAL ${LIST_SIZE_} )
       SET( FFMPEG_FOUND TRUE )
       SET( FFMPEG_SWSCALE_FOUND ${SWSCALE_FOUND} )
    ENDIF ( ${INC_SUCCESS} EQUAL ${LIST_SIZE_} )