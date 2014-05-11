#!/bin/sh
export VIDIOT_DIR=/home/epra/Vidiot
cd ${VIDIOT_DIR}
if [ ! -d Build ]; then
  mkdir Build
fi
cd Build
export PKG_CONFIG_PATH=${VIDIOT_DIR}/ffmpeg/ffmpeg_build/lib/pkgconfig
cmake -G "CodeBlocks - Unix Makefiles" -Wdev --debug-output $HOME/Vidiot/vidiot_trunk

read a

