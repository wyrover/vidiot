set(TESTDIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(../test/CxxTestMacros.inc)

INCLUDE_DIRECTORIES(AFTER
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../test/include
)

macro(AddTests TestProjectName)
    file(GLOB TestHeaders ${TESTDIR}/*/*.h)
    file(GLOB TestSources ${TESTDIR}/*/*.cpp)
    AddCxxTest(${TestProjectName} ${TestHeaders} ${TestSources})

    SOURCE_GROUP("generated" REGULAR_EXPRESSION ${TestRunnerDirectory}/.*)
    SOURCE_GROUP("src" REGULAR_EXPRESSION ${TESTDIR}/src/.*)
    SOURCE_GROUP("include" REGULAR_EXPRESSION ${TESTDIR}/include/.*)

    TARGET_LINK_LIBRARIES(${TestProjectName} test vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})
endmacro(AddTests)

