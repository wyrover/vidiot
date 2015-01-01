# Copyright 2013-2015 Eric Raijmakers.
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

set( CxxTestDir "${PROJECT_SOURCE_DIR}/build/cxxtest-3.10.1" )
set( CxxTestGen "${CxxTestDir}/cxxtestgen.pl" )

macro(AddCxxTest TestProjectName)
  include_directories(AFTER ${CxxTestDir})
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
