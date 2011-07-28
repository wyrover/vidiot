INCLUDE(../test/CxxTestMacros.inc)

SOURCE_GROUP("src"      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/src/.*)
SOURCE_GROUP("include"  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/include/.*)

INCLUDE_DIRECTORIES(AFTER
    include
)

file(GLOB GenericTestFiles ${CMAKE_CURRENT_SOURCE_DIR}/*/*.*)

# Macros are added here to have them include in solution file
ADD_LIBRARY(test ${GenericTestFiles} CxxTestMacros.inc TestMacros.inc)
