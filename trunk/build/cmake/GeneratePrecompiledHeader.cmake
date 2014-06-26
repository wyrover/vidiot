# Usage:
# 1. Create <curdir>/<path>/include/<name>.h
#           <curdir>/<path>/src/<name>.cpp (contents: #include <name>.h)
# 2. Call create_precompiled_header (<project> <name>) to add both the creation
#    of the pdh and the usage of the pch to the given project
# 3. For GCC ensure that the folder of the generated PCH (CMAKE_CURRENT_BINARY_DIR)
#    is in the include list, and before the include dir of the originating .h file.
macro (create_precompiled_header project path name)
  message ("Generating precompiled header for ${project} using ${path}/include/${name}.h")
  set (PCH_FILE "${path}/include/${name}.h")
  include_directories (BEFORE ${path}/include)
  if (MSVC)
    set_target_properties(${project} PROPERTIES COMPILE_FLAGS "/Yu${name}.h -Zm300 /FI${name}.h")
    # todo remaor eset (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI${name}.h")
    #     set_target_properties(${project} PROPERTIES COMPILE_FLAGS "/Yu${name}.h -Zm140")
    #set_target_properties(${project} PROPERTIES COMPILE_FLAGS "/Yu${name}.h -Zm140")
    set_source_files_properties(${path}/src/${name}.cpp PROPERTIES COMPILE_FLAGS "/Yc${name}.h")
  else ()
    set_target_properties (${project} PROPERTIES COMPILE_FLAGS "-include ${name}.h")

    string (TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" build_flags)
    set (compiler_flags ${${build_flags}})

    foreach (item ${CMAKE_CXX_FLAGS})
      list (APPEND compiler_flags "${item}")
    endforeach ()

    get_directory_property (directory_flags INCLUDE_DIRECTORIES)
    foreach (item ${directory_flags})
      list (APPEND compiler_flags "-I${item}")
    endforeach ()

     # Get the list of all build-independent preprocessor definitions
    get_directory_property (defines_global COMPILE_DEFINITIONS)
    list (APPEND defines ${defines_global})

    # Get the list of all build-dependent preprocessor definitions
    string (TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" defines_for_build_name)
    get_directory_property (defines_build ${defines_for_build_name})
    list (APPEND defines ${defines_build})

    # Add the "-D" prefix to all of them
    foreach (item ${defines})
      list (APPEND all_define_flags "-D${item}")
    endforeach ()

    list (APPEND compiler_flags ${all_define_flags})

    # Prepare the compile flags var for passing to GCC
    separate_arguments( compiler_flags )

    set (PCH_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h.gch")
    set (PCH_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${path}/include/${name}.h")
#    if ("${name}" STREQUAL "PrecompiledTest")
#    message( FATAL_ERROR ${PCH_SOURCE})
#    endif()
    add_custom_target (${name}_gch DEPENDS ${PCH_OUTPUT})
    add_custom_command (OUTPUT ${PCH_OUTPUT}
                          COMMAND ${CMAKE_CXX_COMPILER} ${compiler_flags} ${PCH_SOURCE} -o ${PCH_OUTPUT}
                          DEPENDS ${PCH_SOURCE}
                          #MAIN_DEPENDENCY ${PCH_SOURCE}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                          VERBATIM)
                          #todo add include path for Precompiled_gch tafget

      add_dependencies (${project} ${name}_gch)
      set_target_properties (${name}_gch PROPERTIES COMPILE_FLAGS "-include ${name}.h -Winvalid-pch")
    endif ()
endmacro ()

# Must be macro to 'expose' PCH_FILE variable
macro (reuse_precompiled_header project path name)
  set (PCH_FILE "${path}/include/${name}.h")
  include_directories (BEFORE ${path}/include)
  message ("Reusing precompiled header for ${project} using ${PCH_FILE}")
  if (MSVC)
    set_target_properties(${project} PROPERTIES COMPILE_FLAGS "/FI${name}.h")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI${PCH_FILE} ") # todo whats the diff between this method and the gcc PROPERTIES COMPILE_FLAGS  method? choose one
  else ()
    set_target_properties(${project} PROPERTIES COMPILE_FLAGS "-include ${PCH_FILE}")
  endif ()
endmacro ()


# todo set include directories to ${path} BEFORE (avoid having to add ../pch/include in test cmakelists)