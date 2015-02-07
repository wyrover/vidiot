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
export VIDIOT_DIR=${HOME}/Vidiot
export SOURCE=${VIDIOT_DIR}/vidiot_trunk
export BUILD=${VIDIOT_DIR}/Build
export BUILD_DEBUG=${BUILD}/Debug
export BUILD_RELEASE=${BUILD}/Release

export BOOSTROOT=${VIDIOT_DIR}/boost/install

LinuxSetup()
{
	if [ -h ${VIDIOT_DIR} ]; then
		target=`ls -l ${VIDIOT_DIR} | awk '{print $11}'`
		if [ ! ${target} = /media/${USER}/Vidiot ]; then
			echo "Link ${VIDIOT_DIR} points to ${target}."
			read
			exit
		fi
	else
	    ln -s /media/${USER}/Vidiot ${HOME}/Vidiot
	fi
	if [ ! -h ${VIDIOT_DIR}/vidiot_trunk ]; then
	    ln -s /media/sf_vidiot_trunk ${VIDIOT_DIR}/vidiot_trunk # This helps keeping the paths in codeblocks shorter, otherwise codeblocks lists paths as ../../../media/sf_vidiot_trunk/...
	fi
    gsettings set com.canonical.desktop.interface scrollbar-mode normal # fix nasty ubuntu scrollbars
    sudo add-apt-repository -y ppa:webupd8team/sublime-text-3
    sudo apt-get -y update
    # General
    sudo apt-get -y install joe ubuntu-tweak-tool unity-tweak-tool subversion sublime-text-installer openjdk-7-jdk libsaxon* codeblocks codeblocks-contrib cmake cmake-qt-gui g++ portaudio19-dev libsoundtouch-dev
    # wxwidgets - dos2unix required since windows checkout of wxwidgets gives the wx-config utility dos line endings, making it fail to run.
    sudo apt-get -y install libgtk2.0-dev dos2unix 
    # ffmpeg
    sudo apt-get -y install autoconf automake build-essential libass-dev libfreetype6-dev libgpac-dev libsdl1.2-dev libtheora-dev libtool libva-dev libvdpau-dev libvorbis-dev libx11-dev libxext-dev libxfixes-dev pkg-config texi2html zlib1g-dev yasm libmp3lame-dev libopus-dev
    # boost
    sudo apt-get -y install python-dev autotools-dev libicu-dev libbz2-dev libzip-dev
}

BoostSetup()
{
    # Parts taken from http://particlephysicsandcode.com/2013/03/11/installing-boost-1-52-ubuntu-12-04-fedora
    # Matthew M Reid 10/01/2013.
    export BOOSTVER="56"
    if [ ! -d ${VIDIOT_DIR} ]; then 
        return 
    fi
    export BOOSTDIR=${VIDIOT_DIR}/boost
    if [ -d ${BOOSTDIR} ]; then
        rm -rf ${BOOSTDIR}
    fi
    mkdir -p ${BOOSTDIR}/src
    mkdir -p ${BOOSTDIR}/install

    # Extract
    cd ${BOOSTDIR}/src
    wget -O boost_1_${BOOSTVER}_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.${BOOSTVER}.0/boost_1_${BOOSTVER}_0.tar.gz/download
    tar xzvf *.tar.gz
    cd boost_*/

    # Build
    ./bootstrap.sh --prefix=${BOOSTDIR}/install
    n=`cat /proc/cpuinfo | grep "cpu cores" | uniq | awk '{print $NF}'`
    ./b2 --with=all -j $n cxxflags="-std=c++11" --target=shared,static install
    sudo echo "${BOOSTDIR}/install/lib" > /etc/ld.so.conf.d/boost-1.56.0.conf
    sudo ldconfig -v
}

WxSetup()
{
    # See: http://wiki.wxwidgets.org/Compiling_and_getting_started
    if [ ! -d ${VIDIOT_DIR} ]; then 
    	return 
    fi
    export WXDIR=${VIDIOT_DIR}/wxwidgets
    if [ -d ${WXDIR} ]; then
    	rm -rf ${WXDIR}
    fi
    mkdir -p ${WXDIR}/build
    mkdir -p ${WXDIR}/install
    cd ${WXDIR}/build
    /media/sf_wxwidgets_trunk/configure --enable-debug --enable-debug_info --enable-debug_gdb --prefix=${WXDIR}/install \
        --enable-unicode --disable-shared --disable-compat28 --enable-dataviewctrl
    make
    make install
}

FfmpegSetup()
{
    # http://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu
    export FFDIR=${VIDIOT_DIR}/ffmpeg
    export FFBUILD=${FFDIR}/ffmpeg_build
    export FFSRC=${FFDIR}/ffmpeg_sources
    if [ ! -d ${VIDIOT_DIR} ]; then 
    	return 
    fi
    cd ${VIDIOT_DIR}
    if [ -d ffmpeg ]; then
    	rm -rf ffmpeg
    fi
    mkdir -p ${FFBUILD}
    mkdir -p ${FFSRC}
    # libx264:
    cd ${FFSRC}
    wget http://download.videolan.org/pub/x264/snapshots/last_x264.tar.bz2
    tar xjvf last_x264.tar.bz2
    cd x264-snapshot*
    read dummy
    ./configure --prefix="${FFBUILD}" --bindir="${FFDIR}/bin" --enable-static
    make
    make install
    make distclean
    # libfdk-aac:
    cd ${FFDIR}/ffmpeg_sources
    wget -O fdk-aac.zip https://github.com/mstorsjo/fdk-aac/zipball/master
    unzip fdk-aac.zip
    cd mstorsjo-fdk-aac*
    autoreconf -fiv
    ./configure --prefix="${FFBUILD}" --disable-shared
    make
    make install
    make distclean
    # libvpx:
    cd ${FFDIR}/ffmpeg_sources
    wget http://webm.googlecode.com/files/libvpx-v1.3.0.tar.bz2
    tar xjvf libvpx-v1.3.0.tar.bz2
    cd libvpx-v1.3.0
    ./configure --prefix="${FFBUILD}" --disable-examples
    make
    make install
    make clean
    # ffmpeg:
    cd ${FFDIR}/ffmpeg_sources
    wget http://ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2
    tar xjvf ffmpeg-snapshot.tar.bz2
    cd ${FFDIR}
    PKG_CONFIG_PATH="${FFBUILD}/lib/pkgconfig"
    export PKG_CONFIG_PATH
    ./configure --prefix="${FFBUILD}" --extra-cflags="-I${FFBUILD}/include" \
       --extra-ldflags="-L${FFBUILD}/lib" --bindir="${FFDIR}/bin" --extra-libs="-ldl" --enable-gpl \
       --enable-libass --enable-libfdk-aac --enable-libfreetype --enable-libmp3lame --enable-libopus \
       --enable-libtheora --enable-libvorbis --enable-libvpx --enable-libx264 --enable-nonfree --enable-x11grab
    make
    make install
    make distclean
    hash -r
}

Icons()
{
cat > "${HOME}/Desktop/Run CMake.desktop" <<DELIM
#!/usr/bin/env xdg-open
[Desktop Entry]
Version=1.0
Type=Application
Terminal=false
Icon[en_US]=gnome-panel-launcher
Name[en_US]=RunCMake
Exec=gnome-terminal --execute /home/epra/Vidiot/vidiot_trunk/build/RunCMake.sh
Name=Run CMake
Icon=/usr/share/pixmaps/cmake.xpm
DELIM
cat > "${HOME}/Desktop/Clean and run CMake.desktop" <<DELIM
#!/usr/bin/env xdg-open
[Desktop Entry]
Version=1.0
Type=Application
Terminal=false
Icon[en_US]=gnome-panel-launcher
Name[en_US]=Clean and run CMake
Exec=gnome-terminal --execute /home/epra/Vidiot/vidiot_trunk/build/RunCMake.sh REBUILD
Name=RunCMake
Icon=/usr/share/pixmaps/cmake.xpm
DELIM
    chmod a+x "${HOME}/Desktop/Run CMake.desktop"
    chmod a+x "${HOME}/Desktop/Clean and run CMake.desktop"
}

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
    cd ${VIDIOT_DIR}/wxwidgets/install/lib/wx/
    find . -type f | xargs dos2unix

}

MakeDirs()
{
    cd ${VIDIOT_DIR}
    if [ ! -d Build ]; then 
        mkdir Build
        echo "\n----------------------------------------\nCreate directory structure...\n"
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
    mkdir -p ${BUILD_DEBUG}
    cd ${BUILD_DEBUG}
    echo "\n----------------------------------------\nRunCMake DEBUG...\n"
    cmake -G "CodeBlocks - Unix Makefiles" -Wdev ${SOURCE} -DCMAKE_BUILD_TYPE:STRING="DEBUG" #--debug-output
    cd ${BUILD_RELEASE}
    echo "\n----------------------------------------\nRunCMake RELEASE...\n"
    cmake -G "CodeBlocks - Unix Makefiles" -Wdev ${SOURCE} -DCMAKE_BUILD_TYPE:STRING="RELEASE"
    mkdir -p ${BUILD_RELEASE}
}

Rebuild()
{
	Clean 
	Prepare
	MakeDirs
	MakeReadme
	RunCMake
}

case $1 in
FULLSETUP) LinuxSetup ; Icons ; BoostSetup ; FfmpegSetup ; WxSetup ; Rebuild ;;
LINUXSETUP) LinuxSetup ;;
ICONS) Icons ;;
BOOSTSETUP) BoostSetup ;;
WXSETUP) WxSetup ;;
FFMPEGSETUP) FfmpegSetup ;;
REBUILD) Rebuild ;;
DELIVER) echo "Deliver not implemented yet..." ;;
*)       MakeDirs ; MakeReadme ; RunCMake ;;
esac

echo "\n----------------------------------------\nTotal running time: $(date -d @$(($(date +%s)-$start)) +"%M minutes %S seconds")\n"
read a
