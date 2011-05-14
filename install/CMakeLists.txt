
INCLUDE(InstallRequiredSystemLibraries)

# todo: http://www.vtk.org/Wiki/CMake:CPackPackageGenerators#NSIS_Generator_Specifics_settings
SET(APPNAME Vidiot)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${APPNAME})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${APPNAME})

SET(CPACK_PACKAGE_VENDOR "Eric Raijmakers")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Readme.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "3")
SET(CPACK_PACKAGE_EXECUTABLES main;Vidiot)
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
#  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Vidiot.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${APPNAME}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\sourceforge.net\\\\projects\\\\vidiot")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\sourceforge.net\\\\projects\\\\vidiot")
  SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
  
   install(FILES ${FFMPEG_INSTALL} DESTINATION bin)
   install(FILES ${PORTAUDIO_INSTALL} DESTINATION bin)
   install(FILES ${SOUNDTOUCH_INSTALL} DESTINATION bin)
   install(FILES ${BOOST_INSTALL} DESTINATION bin)
    
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/Vidiot")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)
