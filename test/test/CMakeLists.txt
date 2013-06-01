INCLUDE(../test/CxxTestMacros.inc)

SOURCE_GROUP("src"      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/src/.*)
SOURCE_GROUP("include"  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/include/.*)

INCLUDE_DIRECTORIES(AFTER
    include
)

file(GLOB GenericTestFilesH ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)
file(GLOB GenericTestFilesCpp ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp)

ENABLE_UNITY_BUILD(test GenericTestFilesCpp TEST_UNITY_CPP_FILES)

# Macros are added here to have them include in solution file
ADD_LIBRARY(test ${TEST_UNITY_CPP_FILES} ${GenericTestFilesH} ${GenericTestFilesCpp} CxxTestMacros.inc TestMacros.inc)
