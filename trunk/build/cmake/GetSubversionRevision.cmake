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

# Source:
# http://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-svn-revision

# the FindSubversion.cmake module is part of the standard distribution
include(FindSubversion)
# extract working copy information for SOURCE_DIR into MY_XXX variables

Subversion_WC_INFO(${SOURCE_DIR} MY)
message( "Current revision:  ${MY_WC_REVISION}" )
# write a file with the SVNVERSION define
file(WRITE SubversionRevision.h.txt "#define SubversionRevision ${MY_WC_REVISION}\n")
# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different SubversionRevision.h.txt SubversionRevision.h)
