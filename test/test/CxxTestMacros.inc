set( CxxTestDir "${PROJECT_SOURCE_DIR}/build/cxxtest-3.10.1" )
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

endmacro(AddCxxTest)