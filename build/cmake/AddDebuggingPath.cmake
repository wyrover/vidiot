# Fill this variable with extra dll paths required for running the application
# in a debugging context. The variable is used to set MSVC debugging paths.
# This avoids having to change the system path for running with dlls from E.g.
# ffmpeg, boost, and portaudio in the debugger.
set (DEBUGGING_PATH "")
set (MSVC_USER_TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/build/template.vcxproj.user)
macro (add_debugging_path dllpath)
  if (MSVC)
    set (DEBUGGING_PATH ${DEBUGGING_PATH};${dllpath})
  endif ()
endmacro ()
macro (copy_msvc_project_user_file projectname)
  configure_file (${MSVC_USER_TEMPLATE_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${projectname}.vcxproj.user @ONLY)
endmacro ()
