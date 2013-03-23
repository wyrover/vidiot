set( CxxTestDir "${VIDIOT_DIR}/cxxtest-3.10.1" )
set( CxxTestGen "${CxxTestDir}/cxxtestgen.pl" )

INCLUDE_DIRECTORIES(AFTER
    ${CxxTestDir}
)

macro(AddCxxTest TestProjectName)
    set(TestRunnerDirectory ${CMAKE_CURRENT_BINARY_DIR}/${TestProjectName})
    set(TestRunner ${TestRunnerDirectory}/TestRunner.cpp)
    set(TestRunnerCommand ${EXECUTABLE_OUTPUT_PATH}/${TestProjectName})
    add_custom_command(OUTPUT ${TestRunner}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TestRunnerDirectory}
        COMMAND ${CxxTestGen} --runner=ParenPrinter -o ${TestRunner} ${ARGN}
        DEPENDS ${ARGN}
        WORKING_DIRECTORY ${TESTDIR})
        # --error-printer --runner=ParenPrinter --gui=Win32Gui 
    add_executable(${TestProjectName} ${TestRunner} ${ARGN})

    # Copy html files to output folder
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_CURRENT_BINARY_DIR}/Debug/html)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_CURRENT_BINARY_DIR}/Release/html)
    # Copy icons to output folder
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Debug/icons)
    add_custom_command(TARGET ${TestProjectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Release/icons)

endmacro(AddCxxTest)