# Copyright 2013,2014 Eric Raijmakers.
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

include (../test/CxxTestMacros.inc)

source_group ("src"      REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/src/.*)
source_group ("include"  REGULAR_EXPRESSION ${CMAKE_CURRENT_SOURCE_DIR}/include/.*)

# CMAKE_CURRENT_BINARY_DIR is put in the front to ensure that a gcc PCH is found before the associated .h file.
include_directories (AFTER
  ${CMAKE_CURRENT_BINARY_DIR}
  ${INCLUDE_LIST}
  ${VLD_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
  include
)

file (GLOB GenericTestFilesH ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)
file (GLOB GenericTestFilesCpp ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp)

file (GLOB PchTestTiles ${CMAKE_CURRENT_SOURCE_DIR}/../pch/*/*.*)
source_group ("pch" FILES  ${PchTestTiles})

enable_unity_build (test GenericTestFilesCpp TEST_UNITY_CPP_FILES)

# Macros are added here to have them include in MSVC solution
# Same for PrecompiledTest.*
add_library (test ${TEST_UNITY_CPP_FILES} ${GenericTestFilesH} ${GenericTestFilesCpp} ../pch/include/PrecompiledTest.h ../pch/src/PrecompiledTest.cpp CxxTestMacros.inc TestMacros.inc)

create_precompiled_header ("test" "../pch" "PrecompiledTest")

