
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

Clean()
{
    echo "\n----------------------------------------\nCleaning...\n"
    cd ${VIDIOT_DIR}
    rm -rf Build
}

RunCMake()
{
    # Avoid problems with windows line endings
    cd ${VIDIOT_DIR}/wxWidgets/install/lib/wx/
    find . -type f | xargs dos2unix

    cd ${VIDIOT_DIR}
    if [ ! -d Build ]; then 
        mkdir Build
    fi
    cd ${VIDIOT_DIR}/Build
    if [ ! -d Debug ]; then 
        mkdir Debug
    fi
    cd Debug
    echo "\n----------------------------------------\nRunCMake DEBUG...\n"
    cmake -G "CodeBlocks - Unix Makefiles" -Wdev ${SOURCE} -DCMAKE_BUILD_TYPE:STRING="DEBUG" #--debug-output
    cd ${VIDIOT_DIR}/Build
    if [ ! -d Release ]; then 
        mkdir Release
    fi
    cd Release
    echo "\n----------------------------------------\nRunCMake RELEASE...\n"
    cmake -G "CodeBlocks - Unix Makefiles" -Wdev ${SOURCE} -DCMAKE_BUILD_TYPE:STRING="RELEASE"
}

case $1 in
REBUILD) Clean ; RunCMake ;;
DELIVER) echo "Deliver not implemented yet..." ;;
*)       RunCMake ;;
esac

echo "\n----------------------------------------\nTotal running time: $(date -d @$(($(date +%s)-$start)) +"%M minutes %S seconds")\n"
read a
