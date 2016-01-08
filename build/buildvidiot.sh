#!/bin/sh

# Copyright 2014-2016 Eric Raijmakers.
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
export VIDIOT_DIR=${HOME}/vidiot
export SOURCE=${VIDIOT_DIR}/vidiot_trunk
export BUILD=${VIDIOT_DIR}/build
export BUILD_DEBUG=${BUILD}/debug
export BUILD_RELEASE=${BUILD}/release

export BOOSTROOT=${VIDIOT_DIR}/boost/install
export WXSRC_DIR=/media/sf_wxWidgets
if [ ! -d ${WXSRC_DIR} ]; then
    export WXSRC_DIR=/media/${USER}/System/Vidiot/wxWidgets
fi

LinuxSetup()
{
    if [ -d /media/sf_vidiot_trunk ]; then
        # Running in VirtualBox
        PATH_TO_SOURCES=/media/sf_vidiot_trunk
        if [ -h ${VIDIOT_DIR} ]; then
            target=`ls -l ${VIDIOT_DIR} | awk '{print $11}'`
            if [ ! ${target} = /media/${USER}/Vidiot ]; then
                echo "Link ${VIDIOT_DIR} points to ${target}."
                read
                exit
            fi
        else
            ln -s /media/${USER}/Vidiot ${VIDIOT_DIR}
        fi
    else
        # Not in Virtualbox
        PATH_TO_SOURCES=/media/${USER}/System/Vidiot/vidiot_trunk
        mkdir -p ${VIDIOT_DIR}
    fi
    if [ ! -h ${VIDIOT_DIR}/vidiot_trunk ]; then
        ln -s ${PATH_TO_SOURCES} ${VIDIOT_DIR} # This helps keeping the paths in codeblocks shorter, otherwise codeblocks lists paths as ../../../media/sf_vidiot_trunk/...
    fi
    # gsettings set com.canonical.desktop.interface scrollbar-mode normal # fix ubuntu scrollbars
    sudo add-apt-repository -y ppa:webupd8team/sublime-text-3
    sudo apt-get -y update
    echo "\n----------------------------------------\nGeneral...\n"
    sudo apt-get -y install joe \
    						nautilus-open-terminal \
    						unity-tweak-tool \
    						subversion \
    						apt-file \
    						sublime-text-installer \
    						openjdk-7-jdk \
    						libsaxon* \
    						codeblocks codeblocks-contrib \
    						cmake cmake-qt-gui \
    						g++ \
    						portaudio19-dev \
    						libsoundtouch-dev \
                            meld nautilus-compare
    nautilus -q
    echo "\n----------------------------------------\nwx...\n"
    # wxwidgets - dos2unix required since windows checkout of wxwidgets gives the wx-config utility dos line endings, making it fail to run.
#    sudo add-apt-repository -y 'http://repos.codelite.org/wx3.0.2/ubuntu/ universe'	
#    sudo apt-get -y update
    sudo apt-get -y install libgtk2.0-dev dos2unix gettext poedit
#    sudo apt-get install libwxbase3.0-0-unofficial \
#		                 libwxbase3.0-dev \#
#		                 libwxgtk3.0-0-unofficial \
#		                 libwxgtk3.0-dev \
#		                 wx3.0-headers \
#		                 wx-common \
#		                 libwxbase3.0-dbg \
#		                 libwxgtk3.0-dbg \
#		                 wx3.0-i18n \
#		                 wx3.0-examples \
#		                 wx3.0-doc
    echo "\n----------------------------------------\nffmpeg...\n"
    sudo apt-get -y install autoconf \
    						automake \
    						build-essential \
    						libass-dev \
    						libfreetype6-dev \
    						libgpac-dev \
    						libtheora-dev \
    						libtool \
                            libva-dev \
    						libvorbis-dev \
    						pkg-config \
    						texi2html \
    						zlib1g-dev \
    						yasm \
    						libmp3lame-dev \
    						libopus-dev \
    						libx264-dev \
    						mercurial
    echo "\n----------------------------------------\nboost...\n"
    sudo apt-get -y install python-dev autotools-dev libicu-dev libbz2-dev libzip-dev
}

BoostSetup()
{
    # Parts taken from http://particlephysicsandcode.com/2013/03/11/installing-boost-1-52-ubuntu-12-04-fedora
    # Matthew M Reid 10/01/2013.
    export BOOSTVER="59"
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
    echo "${BOOSTDIR}/install/lib" | sudo tee /etc/ld.so.conf.d/boost-1.56.0.conf
    sudo ldconfig -v
}

WxSetup()
{
    # See: http://wiki.wxwidgets.org/Compiling_and_getting_started
    if [ ! -d ${VIDIOT_DIR} ]; then 
    	return 
    fi
    export WXDIR=${VIDIOT_DIR}/wxWidgets_linuxbuild
    if [ -d ${WXDIR} ]; then
    	rm -rf ${WXDIR}
    fi
    mkdir -p ${WXDIR}/build
    mkdir -p ${WXDIR}/install
    cd ${WXDIR}/build
    ${WXSRC_DIR}/configure --enable-debug --enable-debug_info --enable-debug_gdb --prefix=${WXDIR}/install \
        --enable-unicode --disable-shared --disable-compat28 --enable-dataviewctrl
    make
    cd ${WXSRC_DIR}
    make allmo
    cd -
    cd ${WXDIR}/build
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
    # x265/hevc:
    cd ${FFSRC}
	hg clone https://bitbucket.org/multicoreware/x265
	cd ${FFSRC}/x265/build/linux
	PATH="$HOME/bin:$PATH" cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="${FFBUILD}" -DENABLE_SHARED:bool=off ../../source
	make
	make install
	make distclean
    # libfdk-aac:
    cd ${FFSRC}
    wget -O fdk-aac.zip https://github.com/mstorsjo/fdk-aac/zipball/master
    unzip fdk-aac.zip
    cd mstorsjo-fdk-aac*
    autoreconf -fiv
    ./configure --prefix="${FFBUILD}" --disable-shared
    make
    make install
    make distclean
    # libvpx:
    cd ${FFSRC}
    wget http://storage.googleapis.com/downloads.webmproject.org/releases/webm/libvpx-1.4.0.tar.bz2
    tar xjvf libvpx-1.4.0.tar.bz2
    cd libvpx-1.4.0
    ./configure --prefix="${FFBUILD}" --disable-examples  --disable-unit-tests
    make
    make install
    make clean
    # ffmpeg:
    cd ${FFSRC}
    wget http://ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2
    tar xjvf ffmpeg-snapshot.tar.bz2
    cd ffmpeg
    PKG_CONFIG_PATH="${FFBUILD}/lib/pkgconfig"
    export PKG_CONFIG_PATH
    ./configure --prefix="${FFBUILD}" --pkg-config-flags="--static" --extra-cflags="-I${FFBUILD}/include" \
       --extra-ldflags="-L${FFBUILD}/lib" --bindir="${FFDIR}/bin" --enable-gpl \
       --enable-libass --enable-libfdk-aac --enable-libfreetype --enable-libmp3lame --enable-libopus \
       --enable-libtheora --enable-libvorbis --enable-libvpx --enable-libx264 --enable-libx265 --enable-nonfree
    make
    make install
    make distclean
    hash -r
}

Icon()
{
cat > "${HOME}/.Desktop/V_${1}.desktop" <<DELIM
#!/usr/bin/env xdg-open
[Desktop Entry]
Version=1.0
Type=Application
Terminal=false
Icon[en_US]=${SOURCE}/images/movie48.png
Name[en_US]=V_${1}
Exec=gnome-terminal --geometry 100x50+0+0 --execute ${0} ${1}
Name=Vidiot Menu
Icon=/usr/share/pixmaps/cmake.xpm
DELIM
  chmod a+x "${HOME}/.Desktop/V_${1}.desktop"
}

Icons()
{
    Icon Menu
}

Clean()
{
    echo "\n----------------------------------------\nCleaning...\n"
    cd ${VIDIOT_DIR}
    rm -rf build
}

FixWxLineEndings()
{
    echo "\n----------------------------------------\nPrepare...\n"
    # Avoid problems with windows line endings
    cd ${VIDIOT_DIR}/wxwidgets_linuxbuild/install/lib/wx/
    find . -type f | xargs dos2unix
}

UpdateLanguageFiles()
{
    languages_root=${SOURCE}/lang
    find -L ${SOURCE} -name *.cpp -o -name *.h > /tmp/files
	echo Updating template lang/vidiot.pot
	xgettext -f /tmp/files --output=${languages_root}/vidiot.pot --keyword=_ --add-comments=" TRANSLATORS"

    for subdir in `ls -d ${languages_root}/*/`; do
    	language=`basename ${subdir}`
    	languagedir=${languages_root}/${language}
        languageonly=`echo ${language} | cut -c1,2`
    	echo
        echo Language: ${language} \(${languageonly}\)

		if [ -f ${languagedir}/vidiot.po ]; then

            echo Updating lang/${language}/vidiot.po from template
            msgmerge -U ${languagedir}/vidiot.po --no-fuzzy-matching ${languages_root}/vidiot.pot

	        echo Compiling lang/${language}/vidiot.po
	        msgfmt ${languagedir}/vidiot.po --output-file=${languagedir}/vidiot.mo

			wxlang_out=${languagedir}/vidiotwx.po
            wxlang_inp=${WXSRC_DIR}/locale/${language}.po
            if [ ! -f ${wxlang_inp} ]; then
                # from nl_NL take 'nl'
                wxlang_inp=${WXSRC_DIR}/locale/${languageonly}.po
            fi

			if [ -f ${wxlang_out} ]; then
				if [ -f ${wxlang_inp} ]; then

					echo Updating ${wxlang_out}
					msgmerge -U ${wxlang_inp} --no-fuzzy-matching ${wxlang_out}

					echo Compiling ${wxlang_out}
					msgfmt ${wxlang_out} --output-file=${languagedir}/vidiotwx.mo
				fi
			fi
		fi
   	done
   	# Delete all po~ files
   	find ${languages_root} -name *~  -exec rm -f '{}'  \;

}

MakeDirs()
{
    cd ${VIDIOT_DIR}
    if [ ! -d build ]; then 
        mkdir build
        echo "\n----------------------------------------\nCreate directory structure...\n"
    fi
    cd ${BUILD}
    if [ ! -d debug ]; then 
        mkdir debug
    fi
    cd ${BUILD}
    if [ ! -d release ]; then 
        mkdir release
    fi
}

MakeChangelog()
{
    echo "\n----------------------------------------\nUpdate svn version and get revision log...\n"
    XML_SOURCE=${BUILD}/revisionlog.xml
    if [ ! -f ${XML_SOURCE} ]; then
      svn log ${SOURCE} --xml -r 1200:BASE > ${XML_SOURCE}
    fi

    echo "\n----------------------------------------\nCreate Changelog.txt...\n"
    if [ ! -f ${BUILD_DEBUG}/Changelog.txt ]; then
        saxonb-xslt -s:${XML_SOURCE} -xsl:${SOURCE}/build/Changelog_txt.xslt -o:${BUILD_DEBUG}/Changelog.txt
        cp ${BUILD_DEBUG}/Changelog.txt ${BUILD_RELEASE}/Changelog.txt
        cp ${BUILD_DEBUG}/Changelog.txt ${BUILD_RELEASE}/README.txt
    fi
    echo "\n----------------------------------------\nCreate changelog.gz...\n"
    if  [ ! -f ${BUILD_RELEASE}/changelog.gz ]; then
        cd ${BUILD_RELEASE}
        cp Changelog.txt changelog
        gzip -9 changelog
    fi

    echo "\n----------------------------------------\nCreate changelog.html...\n"
    if [ ! -f ${BUILD_DEBUG}/changelog.html ]; then
        saxonb-xslt -s:${XML_SOURCE} -xsl:${SOURCE}/build/Changelog_htm.xslt -o:${BUILD_DEBUG}/changelog.html
        cp ${BUILD_DEBUG}/changelog.html ${BUILD_RELEASE}/changelog.html
    fi
    echo "\n----------------------------------------\nCreate knownproblems.html...\n"
    python "${SOURCE}/build/make_knownproblems.py" "${BUILD_DEBUG}/knownproblems.html" Linux
    cp ${BUILD_DEBUG}/knownproblems.html ${BUILD_RELEASE}/knownproblems.html
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
	FixWxLineEndings
	MakeDirs
	UpdateLanguageFiles
	MakeChangelog
	RunCMake
}

BuildPackage()
{
    cd ${BUILD_RELEASE}
    codeblocks --target=package --profile="batch" --build Vidiot.cbp --no-log
    cd ${BUILD_RELEASE}
    package=`ls *.deb | tail -n 1`

    echo "\n----------------------------------------\nFix ${package} permissions...\n"
    # From https://github.com/paralect/robomongo/blob/master/install/linux/fixup_deb.sh.in
    # chmod 644 on md5sums in generated package:
    set -e
    rm -rf fix_up_deb # Remove any previous results
    mkdir fix_up_deb
    dpkg-deb -x ${package} fix_up_deb
    dpkg-deb --control ${package} fix_up_deb/DEBIAN
    rm ${package}
    chmod 0644 fix_up_deb/DEBIAN/md5sums
#    chmod 0755 fix_up_deb/DEBIAN/postinst
#    chmod 0755 fix_up_deb/DEBIAN/postrm
    find -type d -print0 |xargs -0 chmod 0755
    fakeroot dpkg -b fix_up_deb ${package}

    echo "\n----------------------------------------\nValidating desktop file...\n"
    desktop-file-validate ${SOURCE}/install/linux/vidiot.desktop 

    echo "\n----------------------------------------\nRun lintian on generated package ${package}...\n"
    lintian "${package}"

    # Note: code after lintian doesn't seem to run when inside this function.
}

Install()
{
    cd ${BUILD_RELEASE}
    package=`ls *.deb | tail -n 1`
    echo "\n----------------------------------------\nInstall generated package ${package}...\n"
    sudo dpkg --install ${package}
}

Exec()
{
    ${1} | tee /tmp/${1}.log
}

action="${1}"
if [ "$action" = "" ] || [ "$action" = "Menu" ]; then
  action=`zenity --list --height=400 --width=270 \
          --hide-column="1"  \
          --print-column="1" \
          --hide-header \
          --title="Select action" \
          --text="Select action to be executed" \
          --column="Action" --column="Description" \
            LINUXSETUP      "Setup linux (folders, tools, icons)" \
            BOOSTSETUP      "Rebuild boost" \
            WXSETUP         "Rebuild wxwidgets" \
            FFMPEGSETUP     "Rebuild ffmpeg" \
            \               "" \
            UPDATELANGUAGE  "Update language files" \
            RUNCMAKE        "CMake only" \
            BUILDPACKAGE    "Build package" \
            REBUILD         "Rebuild" \
            DELIVER         "Rebuild & Install" | cut -f 1 -d '|'`
fi

case ${action} in
LINUXSETUP)     Exec LinuxSetup ; Exec Icons ;;
ICONS)          Exec Icons ;;
FULLSETUP)      Exec LinuxSetup ; Exec Icons ; Exec BoostSetup ; Exec WxSetup ; Exec FfmpegSetup ;;
BOOSTSETUP)     Exec BoostSetup ;;
WXSETUP)        Exec WxSetup ;;
FFMPEGSETUP)    Exec FfmpegSetup ;;
UPDATELANGUAGE) Exec UpdateLanguageFiles ;;
RUNCMAKE)       Exec RunCMake ;;
REBUILD)        Exec Rebuild ;;
BUILDPACKAGE)   Exec BuildPackage ;;
DELIVER)        Exec Rebuild ; Exec BuildPackage  ; Exec Install ;;
*)              exit ;;
esac

echo "\n----------------------------------------\nTotal running time: $(date -d @$(($(date +%s)-$start)) +"%M minutes %S seconds")\n"
read a
