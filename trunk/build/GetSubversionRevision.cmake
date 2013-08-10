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