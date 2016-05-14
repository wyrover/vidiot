# Copyright 2013-2016 Eric Raijmakers.
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

# Add install rules for required system runtimes such as MSVCRxx.dll
set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
include (InstallRequiredSystemLibraries)

set (APPNAME Vidiot)
string (TOLOWER ${APPNAME} APPNAME_LOWER)

#set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Non-linear video editor for home video editing.")
set (CPACK_PACKAGE_VENDOR               "Eric Raijmakers")
set (CPACK_PACKAGE_CONTACT              "Eric Raijmakers <epra@users.sourceforge.net>")
set (CPACK_RESOURCE_FILE_LICENSE        "${CMAKE_CURRENT_SOURCE_DIR}/../License.txt")
set (CPACK_PACKAGE_ICON                 "${CMAKE_SOURCE_DIR}\\\\images\\\\movie150x57.bmp") # Installer header icon: 150x57px, 32bit A8R8G8B8, Don't write color space info (GIMP).
set (CPACK_PACKAGE_VERSION_MAJOR        "0")
set (CPACK_PACKAGE_VERSION_MINOR        "3")
set (CPACK_PACKAGE_VERSION_PATCH        "17" )

file (WRITE "${PROJECT_BINARY_DIR}/VidiotVersion.h.txt" "#define VidiotVersion \"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}\"\n")
execute_process (COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_BINARY_DIR}/VidiotVersion.h.txt" "${PROJECT_BINARY_DIR}/VidiotVersion.h") # Reduces needless rebuilds

set (CPACK_PACKAGE_EXECUTABLES "" "") # See http://public.kitware.com/Bug/view.php?id=7828

if (WIN32 AND NOT UNIX)
  set (CPACK_GENERATOR                  "NSIS;ZIP")
  set (CPACK_PACKAGE_INSTALL_DIRECTORY  ${APPNAME})
  set (CPACK_NSIS_MENU_LINKS            "${APPNAME}.exe" "${APPNAME}")
  
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  # NOTE: movie_all.ico generated via http://iconverticons.com/online/
  set (CPACK_NSIS_INSTALLED_ICON_NAME "${APPNAME}.exe") # Icon used in windows control panel (add/remove programs)
  set (CPACK_NSIS_DISPLAY_NAME        "${APPNAME}")
  set (CPACK_NSIS_HELP_LINK           "http:\\\\\\\\sourceforge.net\\\\projects\\\\vidiot")
  set (CPACK_NSIS_URL_INFO_ABOUT      "http:\\\\\\\\sourceforge.net\\\\projects\\\\vidiot")
  set (CPACK_NSIS_CONTACT             "${CPACK_PACKAGE_CONTACT}")
  set (CPACK_NSIS_MODIFY_PATH         OFF)
  set (CPACK_NSIS_MUI_ICON            "${CMAKE_SOURCE_DIR}\\\\images\\\\movie_all.ico") #  Required for icon on installer AND uninstaller
  set (CPACK_NSIS_MUI_UNIICON         "${CMAKE_SOURCE_DIR}\\\\images\\\\movie_all.ico")  #  Required for icon on installer AND uninstaller

   set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!include FileAssoc.nsh
      !insertmacro APP_ASSOCIATE 'vid' '${APPNAME_LOWER}.projectfile' '${APPNAME} project file' '$INSTDIR\\\\${APPNAME}.exe,0' 'Open with ${APPNAME}' '$INSTDIR\\\\${APPNAME}.exe \\\"%1\\\"'
      System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'"
   )

   set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "!insertmacro APP_UNASSOCIATE 'vid' '${APPNAME_LOWER}.projectfile'
       System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'"
   )

  install (FILES ${FFMPEG_INSTALL}                                              COMPONENT ${APPNAME} DESTINATION .)
  install (FILES ${PORTAUDIO_INSTALL}                                           COMPONENT ${APPNAME} DESTINATION .)
  install (FILES ${BOOST_INSTALL}                                               COMPONENT ${APPNAME} DESTINATION .      CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
  install (FILES ${BOOST_INSTALL_DEBUG}                                         COMPONENT ${APPNAME} DESTINATION .      CONFIGURATIONS Debug)
  install (FILES ${PROJECT_BINARY_DIR}/main/stripped.pdb                        COMPONENT ${APPNAME} DESTINATION .      CONFIGURATIONS RelWithDebInfo     RENAME ${APPNAME_LOWER}.pdb ) # For generating stack traces the pdb file is distributed
  install (
    DIRECTORY ${PROJECT_SOURCE_DIR}/images/                              
    COMPONENT ${APPNAME} 
	DESTINATION images        
    PATTERN "*.svg" EXCLUDE
	PATTERN "*.rc" EXCLUDE
	PATTERN "*.pdn" EXCLUDE)
  install (DIRECTORY ${PROJECT_SOURCE_DIR}/html/                                COMPONENT ${APPNAME} DESTINATION html)
  install (DIRECTORY ${PROJECT_SOURCE_DIR}/lang/                                COMPONENT ${APPNAME} DESTINATION lang)
  install (FILES ${PROJECT_BINARY_DIR}/changelog.html                           COMPONENT ${APPNAME} DESTINATION html/about)
  install (FILES ${PROJECT_BINARY_DIR}/knownproblems.html                       COMPONENT ${APPNAME} DESTINATION html/about)

  if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        install (FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} COMPONENT ${APPNAME} DESTINATION . PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ)
  endif ()

else ()
  set (CPACK_GENERATOR                      "DEB;TGZ")
  # Used "dpkg -l libgtk*" and "apt-cache depends libgtk2.0-bin" to get the dependency info
  set (CPACK_PACKAGE_INSTALL_DIRECTORY      "${APPNAME_LOWER}")
  set (CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE  1)
  set (CPACK_PACKAGE_NAME                   "${APPNAME_LOWER}")
  set (CPACK_DEBIAN_PACKAGE_NAME            "${APPNAME_LOWER}")
  set (CPACK_DEBIAN_PACKAGE_HOMEPAGE        "https://sourceforge.net/projects/vidiot")
  set (CPACK_DEBIAN_PACKAGE_DESCRIPTION     "Non-linear video editor targeted for home video editing. \\n Supports compositing, scaling, rotating, transitions, titles, etc.")
  set (CPACK_STRIP_FILES                    "${APPNAME_LOWER}")
  set (CPACK_SOURCE_STRIP_FILES             "")
  set (CPACK_SYSTEM_NAME                    "i386")
  set (CPACK_DEBIAN_PACKAGE_SECTION         "video")
  set (CPACK_DEBIAN_PACKAGE_PRIORITY        "optional")
  set (CPACK_DEBIAN_PACKAGE_DEPENDS         "libc6 (>=2.19), libgtk2.0-bin (>= 2.24.23), libopus0 (>= 1.1), libmp3lame0 (>= 3.99.5), libva1 (>= 1.3.0), libsoundtouch0 (>= 1.7.1), libportaudio2 (>= 19)")
# Enable this line to include these scripts again (excluded now because they're empty)
#  set (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA   "${PROJECT_SOURCE_DIR}/install/linux/postinst;${PROJECT_SOURCE_DIR}/install/linux/postrm")
  
  # Install shared files used by application
  install (
    DIRECTORY   ${PROJECT_SOURCE_DIR}/images/
    COMPONENT   ${APPNAME_LOWER}
    DESTINATION share/${APPNAME_LOWER}/images
    PATTERN "*.svg" EXCLUDE
	PATTERN "*.rc" EXCLUDE
	PATTERN "*.pdn" EXCLUDE	)
  install (
    DIRECTORY   ${PROJECT_SOURCE_DIR}/html/
    COMPONENT   ${APPNAME_LOWER}
    DESTINATION share/${APPNAME_LOWER}/html )
  install (
    DIRECTORY   ${PROJECT_SOURCE_DIR}/lang/
    COMPONENT   ${APPNAME_LOWER}
    DESTINATION share/${APPNAME_LOWER}/lang )

  # Install informational files
  install (
    FILES ${PROJECT_BINARY_DIR}/changelog.html
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/${APPNAME_LOWER}/html/about )
  install (
    FILES ${PROJECT_BINARY_DIR}/knownproblems.html
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/${APPNAME_LOWER}/html/about )
  install (
    FILES ${PROJECT_BINARY_DIR}/changelog.gz
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/doc/${APPNAME_LOWER}
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
  install (
    FILES ${PROJECT_SOURCE_DIR}/Copyright.txt
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/doc/${APPNAME_LOWER}
    RENAME copyright
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

  # Install man page
  install (
    FILES ${PROJECT_SOURCE_DIR}/install/linux/vidiot.1.gz
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/man/man1
    RENAME ${APPNAME_LOWER}.1.gz
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
  
  # Register vidiot application, .vid mime type, and icon in Nautilus
  install (
    FILES ${PROJECT_SOURCE_DIR}/install/linux/vidiot.desktop
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/applications
    RENAME ${APPNAME_LOWER}.desktop
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
  install (
    FILES ${PROJECT_SOURCE_DIR}/install/linux/vidiot.xml
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/mime/packages
    RENAME ${APPNAME_LOWER}.xml
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
  install (
    FILES ${PROJECT_SOURCE_DIR}/images/movie48.png
    COMPONENT ${APPNAME_LOWER}
    DESTINATION share/icons/hicolor/48x48/mimetypes
    RENAME application-${APPNAME_LOWER}.png
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endif ()

include (CPack)

#cpack_add_component(Vidiot DISPLAY_NAME displayname INSTALL_TYPES vidiot vidiot2)
#cpack_add_install_type(vidiot DISPLAY_NAME Vidiot)
#cpack_add_install_type(vidiot2 DISPLAY_NAME Vidiot2)


