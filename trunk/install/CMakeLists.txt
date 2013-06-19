# Add install rules for required system runtimes such as MSVCRxx.dll
SET (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
INCLUDE(InstallRequiredSystemLibraries)

# http://www.vtk.org/Wiki/CMake:CPackPackageGenerators#NSIS_Generator_Specifics_settings
SET(APPNAME Vidiot)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${APPNAME})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${APPNAME})
SET(CPACK_PACKAGE_VENDOR "Eric Raijmakers")
SET(CPACK_GENERATOR "NSIS")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Readme.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_EXECUTABLES "" "") # See http://public.kitware.com/Bug/view.php?id=7828
SET(CPACK_NSIS_MENU_LINKS "${APPNAME}.exe" "${APPNAME}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  # NOTE: movie_all.ico generated via http://iconverticons.com/online/
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "${APPNAME}.exe") # Icon used in windows control panel (add/remove programs)
  SET(CPACK_NSIS_DISPLAY_NAME "${APPNAME}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\sourceforge.net\\\\projects\\\\vidiot")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\sourceforge.net\\\\projects\\\\vidiot")
  SET(CPACK_NSIS_CONTACT "epra@users.sourceforge.net")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\icons\\\\movie150x57.bmp") # Icon in the header of the installer: Use 150x57 pixels, 32 bit A8R8G8B8, Do not write color space info (GIMP)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\\\icons\\\\movie_all.ico") #  Required for icon on installer AND uninstaller
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}\\\\icons\\\\movie_all.ico")  #  Required for icon on installer AND uninstaller

   set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!include FileAssoc.nsh
      !insertmacro APP_ASSOCIATE 'vid' 'vidiot.projectfile' 'Vidiot project file' '$INSTDIR\\\\Vidiot.exe,0' 'Open with Vidiot' '$INSTDIR\\\\Vidiot.exe /e \\\"%1\\\"'
      System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'"
   )

   set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "!insertmacro APP_UNASSOCIATE 'vid' 'vidiot.projectfile'
       System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'"
   )

  install(FILES ${FFMPEG_INSTALL} COMPONENT ${APPNAME} DESTINATION .)
  install(FILES ${PORTAUDIO_INSTALL} COMPONENT ${APPNAME} DESTINATION .)
  install(FILES ${SOUNDTOUCH_INSTALL} COMPONENT ${APPNAME} DESTINATION .)
  install(FILES ${BOOST_INSTALL} COMPONENT ${APPNAME} DESTINATION .)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/html/ COMPONENT ${APPNAME} DESTINATION html)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/icons/ COMPONENT ${APPNAME} DESTINATION icons)

  IF (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        INSTALL(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} COMPONENT ${APPNAME} DESTINATION . PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ)
  ENDIF (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)

ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "${APPNAME}")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)
