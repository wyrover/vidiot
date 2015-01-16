
#!/bin/sh

# Copyright 2014-2015 Eric Raijmakers.
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
# along with Vidiot. If not, see <http://www.gnu.org/licenses />.

start=$(date +%s)
export VIDIOT_DIR=/home/epra/Vidiot
export SOURCE=${VIDIOT_DIR}/vidiot_trunk
export BUILD=${VIDIOT_DIR}/Build
export BUILD_DEBUG=${BUILD}/Debug
export BUILD_RELEASE=${BUILD}/Release

Clean()
{
    echo "\n----------------------------------------\nCleaning...\n"
    cd ${VIDIOT_DIR}
    rm -rf Build
}

Prepare()
{
    echo "\n----------------------------------------\nPrepare...\n"
    # Avoid problems with windows line endings
    cd ${VIDIOT_DIR}/wxWidgets/install/lib/wx/
    find . -type f | xargs dos2unix

    echo "\n----------------------------------------\nCreate directory structure...\n"
    cd ${VIDIOT_DIR}
    if [ ! -d Build ]; then 
        mkdir Build
    fi
    cd ${BUILD}
    if [ ! -d Debug ]; then 
        mkdir Debug
    fi
    cd ${BUILD}
    if [ ! -d Release ]; then 
        mkdir Release
    fi
}

MakeReadme()
{
    echo "\n----------------------------------------\nUpdate svn version and get revision log...\n"
    XML_SOURCE=${BUILD}/revisionlog.xml
    if [ ! -f ${XML_SOURCE} ]; then
      svn log ${SOURCE} --xml -r 1200:BASE > ${XML_SOURCE}
    fi

    echo "\n----------------------------------------\nCreate README and history.html...\n"
    if [ ! -f ${BUILD_DEBUG}/README.txt ]; then
        saxonb-xslt -s:${XML_SOURCE} -xsl:${SOURCE}/build/make_readme_txt.xslt -o:${BUILD_DEBUG}/README.txt
        cp ${BUILD_DEBUG}/README.txt ${BUILD_RELEASE}/README.txt
    fi
    if [ ! -f ${BUILD_DEBUG}/history.html ]; then
        saxonb-xslt -s:${XML_SOURCE} -xsl:${SOURCE}/build/make_readme_htm.xslt -o:${BUILD_DEBUG}/history.html
        cp ${BUILD_DEBUG}/history.html ${BUILD_RELEASE}/history.html
    fi
}

RunCMake()
{
    cd ${BUILD_DEBUG}
    echo "\n----------------------------------------\nRunCMake DEBUG...\n"
    cmake -G "CodeBlocks - Unix Makefiles" -Wdev ${SOURCE} -DCMAKE_BUILD_TYPE:STRING="DEBUG" #--debug-output
    cd ${BUILD_RELEASE}
    echo "\n----------------------------------------\nRunCMake RELEASE...\n"
    cmake -G "CodeBlocks - Unix Makefiles" -Wdev ${SOURCE} -DCMAKE_BUILD_TYPE:STRING="RELEASE"
}

case $1 in
REBUILD) Clean ; Prepare ; MakeReadme ; RunCMake ;;
DELIVER) echo "Deliver not implemented yet..." ;;
*)       MakeReadme ; RunCMake ;;
esac

echo "\n----------------------------------------\nTotal running time: $(date -d @$(($(date +%s)-$start)) +"%M minutes %S seconds")\n"
read a
