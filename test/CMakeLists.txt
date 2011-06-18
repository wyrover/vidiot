#### CXXTEST ####

set(TESTDIR ${CMAKE_CURRENT_SOURCE_DIR})

set( CxxTestDir "${VIDIOT_DIR}/cxxtest-3.10.1" )
set( CxxTestGen "${CxxTestDir}/cxxtestgen.pl" )

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
    add_test(${TestProjectName} ${TestRunnerCommand})
endmacro(AddCxxTest)

SOURCE_GROUP("generated" REGULAR_EXPRESSION ${TestRunnerDirectory}/.*)
SOURCE_GROUP("src" REGULAR_EXPRESSION ${TESTDIR}/src/.*)
SOURCE_GROUP("include" REGULAR_EXPRESSION ${TESTDIR}/include/.*)
SOURCE_GROUP("build" FILES CMakeLists.txt ${BuildFiles} ${CmakeModulesFiles})

INCLUDE_DIRECTORIES(AFTER
    include
    ${CxxTestDir}
)

file(GLOB TestHeaders ${TESTDIR}/*/*.h)
file(GLOB TestSources ${TESTDIR}/*/*.cpp)
AddCxxTest(test ${TestHeaders} ${TestSources})
TARGET_LINK_LIBRARIES(test vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})
