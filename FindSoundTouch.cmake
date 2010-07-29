UNUSED

# Original Version from PCSX2 : http://code.google.com/p/pcsx2/

# Try to find SoundTouch
# Once done, this will define
#
# SOUNDTOUCH_FOUND - system has SoundTouch
# SOUNDTOUCH_INCLUDE_DIR - the SoundTouch include directories
# SOUNDTOUCH_LIBRARIES - link these to use SoundTouch

if(SOUNDTOUCH_INCLUDE_DIR AND SOUNDTOUCH_LIBRARIES)
    set(SOUNDTOUCH_FIND_QUIETLY TRUE)
endif(SOUNDTOUCH_INCLUDE_DIR AND SOUNDTOUCH_LIBRARIES)

# include dir
find_path(SOUNDTOUCH_INCLUDE_DIR NAMES SoundTouch.h)

# finally the library itself
find_library(libSoundTouch NAMES SoundTouch SoundTouchD PATH_SUFFIXES soundtouch_trunk/source/SoundTouch/Release soundtouch_trunk/source/SoundTouch/Debug )
set(SOUNDTOUCH_LIBRARIES ${libSoundTouch})

# handle the QUIETLY and REQUIRED arguments and set SOUNDTOUCH_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SoundTouch DEFAULT_MSG SOUNDTOUCH_LIBRARIES SOUNDTOUCH_INCLUDE_DIR)
asdf