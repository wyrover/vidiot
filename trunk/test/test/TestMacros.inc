set(TESTDIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(../test/CxxTestMacros.inc)

INCLUDE_DIRECTORIES(AFTER
    ../pch/include
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../test/include
)

macro(AddTests TestProjectName)
    file(GLOB TestHeaders ${TESTDIR}/*/*.h)
    file(GLOB TestSources ${TESTDIR}/*/*.cpp)

    if (${TestProjectName} STREQUAL "testauto")
        FILE(GLOB pch_include ${TESTDIR}/../pch/include/*.h)
        FILE(GLOB pch_src ${TESTDIR}/../pch//src/*.cpp)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FIPrecompiledTest.h ")
    endif (${TestProjectName} STREQUAL "testauto")

    AddCxxTest(${TestProjectName} ${pch_include} ${pch_src} ${TestHeaders} ${TestSources})

    if (${TestProjectName} STREQUAL "testauto")
        if (MSVC)
            set_target_properties(${TestProjectName} PROPERTIES COMPILE_FLAGS "/YuPrecompiledTest.h")
            set_source_files_properties(${TESTDIR}/../pch/src/PrecompiledTest.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiledTest.h")
        endif(MSVC)
        SOURCE_GROUP("pch\\include" REGULAR_EXPRESSION ../pch/include/.*)
        SOURCE_GROUP("pch\\src" REGULAR_EXPRESSION ../pch/src/.*)
    endif (${TestProjectName} STREQUAL "testauto")

    SOURCE_GROUP("generated" REGULAR_EXPRESSION ${TestRunnerDirectory}/.*)
    SOURCE_GROUP("src" REGULAR_EXPRESSION ${TESTDIR}/src/.*)
    SOURCE_GROUP("include" REGULAR_EXPRESSION ${TESTDIR}/include/.*)

    TARGET_LINK_LIBRARIES(${TestProjectName} test vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

    COPY_MSVC_PROJECT_USER_FILE(${TestProjectName})
endmacro(AddTests)

