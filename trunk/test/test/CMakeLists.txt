# Copyright 2013 Eric Raijmakers.
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
# along with Vidiot. If not, see <http:#www.gnu.org/licenses/>.

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
