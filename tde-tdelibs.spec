# BEGIN SourceDeps(oneline):
BuildRequires(pre): rpm-build-suse-compat
BuildRequires: %_bindir/hspell %_bindir/ispell %_bindir/xmllint binutils-devel libXext-devel libXft-devel libdb4-devel libjpeg-devel libpcsclite libpng-devel perl(DB_File.pm) perl(Shell.pm) perl(Term/ReadLine.pm) pkgconfig(fontconfig) pkgconfig(freetype2) pkgconfig(libr) pkgconfig(libxml-2.0) pkgconfig(lua) pkgconfig(xcomposite) pkgconfig(xrandr) pkgconfig(xrender) valgrind-devel zlib-devel
# END SourceDeps(oneline)
%define suse_version 1550
# see https://bugzilla.altlinux.org/show_bug.cgi?id=10382
%define _localstatedir %_var
#
# spec file for package tdelibs (version R14)
#
# Copyright (c) 2014 Trinity Desktop Environment
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.
#
# Please submit bugfixes or comments via http://www.trinitydesktop.org/
#

# BUILD WARNING:
#  Remove qt-devel and qt3-devel and any kde*-devel on your system !
#  Having KDE libraries may cause FTBFS here !

# TDE variables
%define tde_epoch 2
%if "%{?tde_version}" == ""
%define tde_version 14.1.3
%endif
%define tde_pkg tdelibs
%define tde_prefix /opt/trinity
%define tde_bindir %tde_prefix/bin
%define tde_confdir %_sysconfdir/trinity
%define tde_datadir %tde_prefix/share
%define tde_docdir %tde_datadir/doc
%define tde_includedir %tde_prefix/include
%define tde_libdir %tde_prefix/%_lib
%define tde_tdeappdir %tde_datadir/applications/tde
%define tde_tdedocdir %tde_docdir/tde
%define tde_tdeincludedir %tde_includedir/tde
%define tde_tdelibdir %tde_libdir/trinity

Name: tde-tdelibs
Version: 14.1.3
Release: alt1
Summary: TDE Libraries
Group: Graphical desktop/Other
Url: http://www.trinitydesktop.org/

License: GPL-2.0+

#Vendor:			Trinity Desktop
#Packager:		Francois Andriot <francois.andriot@free.fr>

Source: %name.tar
#Source1:		%name-rpmlintrc

Obsoletes: tdelibs < %tde_version
Provides: tdelibs = %tde_version
Obsoletes: tde-kdelibs < %tde_version
Provides: tde-kdelibs = %tde_version
Obsoletes: tde-kdelibs-apidocs < %tde_version
Provides: tde-kdelibs-apidocs = %tde_version

# Trinity dependencies
BuildRequires: libtqt4-devel = %tde_version
#BuildRequires: tde-arts-devel >= %tde_version
BuildRequires: libdbus-tqt-1-devel >= %tde_version
BuildRequires: libdbus-1-tqt-devel >= %tde_version
BuildRequires: libdbus-1-tqt0
BuildRequires: trinity-filesystem >= %tde_version

#Requires: tde-arts >= %tde_version
Requires: trinity-filesystem >= %tde_version

BuildRequires: tde-cmake >= %tde_version
BuildRequires: gcc-c++
BuildRequires: pkgconfig
BuildRequires: fdupes

%if 0%{?suse_version}
BuildRequires: rpm-build-suse-compat
%endif

# KRB5 support
BuildRequires: libkrb5-devel

# XSLT support
BuildRequires: libxslt-devel

# ALSA support
BuildRequires: libalsa-devel

# IDN support
BuildRequires: libidn-devel

# CUPS support
BuildRequires: libcups-devel

# TIFF support
BuildRequires: libtiff-devel libtiffxx-devel

# OPENSSL support
BuildRequires: libssl-devel

# ACL support
BuildRequires: libacl-devel

# GLIB2 support
BuildRequires: glib2-devel libgio libgio-devel

# LUA support are not ready yet
#BuildRequires:	lua-devel

# LIBART_LGPL support
BuildRequires: libart_lgpl-devel
#BuildRequires: lib64art_lgpl-devel

# ASPELL support
BuildRequires: aspell
BuildRequires: libaspell-devel

# GAMIN support
BuildRequires: libgamin-devel

# PCRE support
BuildRequires: libpcre-devel libpcrecpp-devel

# PCRE2 support
BuildRequires: libpcre2 libpcre2-devel

# INOTIFY support
BuildRequires: inotify-tools-devel
%define with_inotify 1

# BZIP2 support
BuildRequires: bzlib-devel

# UTEMPTER support
BuildRequires: libutempter
BuildRequires: libutempter-devel

# HSPELL support
BuildRequires: libhspell-devel

# JASPER support
BuildRequires: libjasper-devel

# OPENEXR support
BuildRequires: openexr-devel

BuildRequires: libpthread-stubs

# LIBTOOL
BuildRequires: libltdl7-devel libltdl7-devel-static
#BuildRequires: libtool_2.4

#libltdl support
BuildRequires: tde-libltdl-devel

# X11 support
BuildRequires: xorg-proto-devel

# ICEAUTH
Requires: iceauth
BuildRequires: iceauth

# Xorg
Requires: xorg-sdk
BuildRequires: xorg-sdk

# XZ support
BuildRequires: liblzma-devel

# Certificates support
BuildRequires: ca-certificates
Requires: ca-certificates
Requires: openssl

# XRANDR support
%define with_xrandr 1

### New features in TDE R14

# LIBMAGIC support
BuildRequires: libmagic-devel

# NETWORKMANAGER support
BuildRequires: libnm-devel libnm-gir-devel

# UDEV support
BuildRequires: libudev-devel

# HAL support
%if 0%{?rhel} == 5
%define with_hal 1
%endif

# UDISKS2 support
Requires: libudisks2
BuildRequires: libudisks2-devel

# UPOWER support
Requires: upower

# SYSTEMD support
%define with_systemd 1

# PCSCLITE support
BuildRequires: libpcsclite-devel

# PKCS11 support
BuildRequires: libpkcs11-helper-devel

# OPENSC support
BuildRequires: libopensc libopensc-devel

# CRYPTSETUP support
BuildRequires: libcryptsetup-devel

# ATTR support
BuildRequires: libattr-devel

# INTLTOOL support
BuildRequires: intltool

# WEBP support
BuildRequires: libwebp-devel

# libdbus support
BuildRequires: libdbus-devel

# libsystemd support
BuildRequires:libsystemd-devel

#NotFoundRequires
#BuildRequires:libbrotli-devel libfreetype-devel libXdmcp-devel libXrender-devel libXext-devel libffi-devel libXft-devel

#Патчи
Patch0: fixbuild.patch

%description
Libraries for the Trinity Desktop Environment:
TDE Libraries included: tdecore (TDE core library), tdeui (user interface),
kfm (file manager), tdehtmlw (HTML widget), tdeio (Input/Output, networking),
kspell (spelling checker), jscript (javascript), kab (addressbook),
kimgio (image manipulation).

%files
%doc AUTHORS COPYING COPYING-DOCS COPYING.LIB README TODO
%tde_bindir/artsmessage
%tde_bindir/cupsdconf
%tde_bindir/cupsdoprint
%tde_bindir/dcop
%tde_bindir/dcopclient
%tde_bindir/dcopfind
%tde_bindir/dcopobject
%tde_bindir/dcopquit
%tde_bindir/dcopref
%tde_bindir/dcopserver
%tde_bindir/dcopserver_shutdown
%tde_bindir/dcopstart
%tde_bindir/imagetops
%tde_bindir/tdeab2tdeabc
%tde_bindir/kaddprinterwizard
%tde_bindir/tdebuildsycoca
%tde_bindir/tdecmshell
%tde_bindir/tdeconf_update
%tde_bindir/kcookiejar
%tde_bindir/tde-config
%tde_bindir/tde-menu
%tde_bindir/kded
%tde_bindir/tdeinit
%tde_bindir/tdeinit_shutdown
%tde_bindir/tdeinit_wrapper
%tde_bindir/tdesu_stub
%tde_bindir/kdetcompmgr
%tde_bindir/kdontchangethehostname
%tde_bindir/tdedostartupconfig
%tde_bindir/tdefile
%tde_bindir/kfmexec
%tde_bindir/tdehotnewstuff
%tde_bindir/kinstalltheme
%tde_bindir/tdeio_http_cache_cleaner
%tde_bindir/tdeio_uiserver
%tde_bindir/tdeioexec
%tde_bindir/tdeioslave
%tde_bindir/tdeiso_info
%tde_bindir/tdelauncher
%if 0%{?with_elficon}
%tde_bindir/tdelfeditor
%endif
%tde_bindir/tdemailservice
%tde_bindir/tdemimelist
%tde_bindir/tdesendbugmail
%tde_bindir/kshell
%tde_bindir/tdestartupconfig
%tde_bindir/tdetelnetservice
%tde_bindir/tdetradertest
%tde_bindir/kwrapper
%tde_bindir/lnusertemp
%tde_bindir/make_driver_db_cups
%tde_bindir/make_driver_db_lpr
%tde_bindir/meinproc
%tde_bindir/networkstatustestservice
%tde_bindir/start_tdeinit_wrapper
%tde_bindir/checkXML
%tde_bindir/ksvgtopng
%tde_bindir/tdeunittestmodrunner
%tde_bindir/preparetips
%tde_tdelibdir/*
%tde_libdir/lib*.so.*
%tde_libdir/libtdeinit_*.la
%tde_libdir/libtdeinit_*.so
%tde_datadir/applications/tde/*.desktop
%tde_datadir/autostart/tdeab2tdeabc.desktop
%tde_datadir/applnk/tdeio_iso.desktop
%tde_datadir/apps/*
%exclude %tde_datadir/apps/ksgmltools2/
%tde_datadir/emoticons/*
%tde_datadir/icons/crystalsvg/
%tde_datadir/icons/default.tde
%tde_datadir/icons/hicolor/index.theme
%tde_datadir/locale/all_languages
%tde_datadir/mimelnk/*/*.desktop
%tde_datadir/services/*
%tde_datadir/servicetypes/*
%tde_tdedocdir/HTML/en/common/*
%tde_tdedocdir/HTML/en/tdespell/

# Global Trinity configuration
%config(noreplace) %tde_confdir

# Some setuid binaries need special care
%if 0%{?suse_version}
%verify(not mode) %tde_bindir/kgrantpty
%verify(not mode) %tde_bindir/kpac_dhcp_helper
%verify(not mode) %tde_bindir/start_tdeinit
%else
%attr(4711,root,root) %tde_bindir/kgrantpty
%attr(4711,root,root) %tde_bindir/kpac_dhcp_helper
%attr(4711,root,root) %tde_bindir/start_tdeinit
%endif

%config %_sysconfdir/xdg/menus/tde-applications.menu
%config %_sysconfdir/xdg/menus/tde-applications.menu-no-kde

# DBUS stuff, related to TDE hwlib
%if 0%{?with_tdehwlib}
%tde_bindir/tde_dbus_hardwarecontrol
%config %_sysconfdir/dbus-1/system.d/org.trinitydesktop.hardwarecontrol.conf
%_datadir/dbus-1/system-services/org.trinitydesktop.hardwarecontrol.service
%endif

%pre
if [ -d "%tde_datadir/locale/all_languages" ]; then
  rm -rf "%tde_datadir/locale/all_languages"
fi

%package devel
Summary: TDE Libraries (Development files)
Group: Development/C
Requires: %name = %{?epoch:%epoch:}%tde_version-%release

Obsoletes: tdelibs-devel < %{?epoch:%epoch:}%tde_version-%release
Provides: tdelibs-devel = %{?epoch:%epoch:}%tde_version-%release
Obsoletes: tde-kdelibs-devel < %{?epoch:%epoch:}%tde_version-%release
Provides: tde-kdelibs-devel = %{?epoch:%epoch:}%tde_version-%release

Requires: libattr-devel
Requires: intltool
%{?xcomposite_devel:Requires: %xcomposite_devel}
%{?xt_devel:Requires: %xt_devel}

%description devel
This package includes the header files you will need to compile
applications for TDE.

%files devel
%tde_bindir/dcopidl*
%tde_bindir/*config_compiler
%tde_bindir/maketdewidgets
%tde_datadir/apps/ksgmltools2/
%tde_tdeincludedir/*
%tde_libdir/*.la
%tde_libdir/*.so
%tde_libdir/*.a
%exclude %tde_libdir/libtdeinit_*.la
%exclude %tde_libdir/libtdeinit_*.so
%tde_datadir/cmake/tdelibs.cmake
%tde_libdir/pkgconfig/tdelibs.pc

##########

%prep
%setup -n %name

# RHEL 5: remove tdehwlib stuff from include files, to avoid FTBFS in tdebindings
%if 0%{?rhel} == 5
%__subst "tdecore/kinstance.h" \
       -i "tdecore/tdeglobal.h" \
       -e "/#ifdef __TDE_HAVE_TDEHWLIB/,/#endif/d"
%endif

%patch0 -p1

%build
unset QTDIR QTINC QTLIB
export PATH="%tde_bindir:${PATH}"
export PKG_CONFIG_PATH="%tde_libdir/pkgconfig:$PKG_CONFIG_PATH"
export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/usr/lib64/pkgconfig:$PKG_CONFIG_PATH"

if [ -d "/usr/X11R6" ]; then
  export RPM_OPT_FLAGS="${RPM_OPT_FLAGS} -L/usr/X11R6/%_lib -I/usr/X11R6/include"
fi

export TDEDIR="%tde_prefix"

#if ! rpm -E %%cmake|grep -e 'cd build\|cd ${CMAKE_BUILD_DIR:-build}'; then
#  mkdir -p build
#  cd build
#fi
cd %name

export CFLAGS="${RPM_OPT_FLAGS}"
export CXXFLAGS="${RPM_OPT_FLAGS} -Wno-error"

%suse_cmake \
  -DCMAKE_C_FLAGS="${CFLAGS}" \
  -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
  -DCMAKE_SKIP_RPATH=OFF \
  -DCMAKE_SKIP_INSTALL_RPATH=OFF \
  -DCMAKE_INSTALL_RPATH="%tde_libdir" \
  -DCMAKE_NO_BUILTIN_CHRPATH=ON \
  -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DWITH_GCC_VISIBILITY=ON \
  \
  -DCMAKE_INSTALL_PREFIX="%tde_prefix" \
  -DBIN_INSTALL_DIR="%tde_bindir" \
  -DCONFIG_INSTALL_DIR="%tde_confdir" \
  -DDOC_INSTALL_DIR="%tde_docdir" \
  -DINCLUDE_INSTALL_DIR="%tde_tdeincludedir" \
  -DLIB_INSTALL_DIR="%tde_libdir" \
  -DPKGCONFIG_INSTALL_DIR="%tde_libdir/pkgconfig" \
  -DSHARE_INSTALL_PREFIX="%tde_datadir" \
  \
  -DWITH_ALL_OPTIONS=ON \
  -DWITH_ARTS=OFF \
  -DWITH_ALSA=ON \
  -DWITH_LIBART=ON \
  -DWITH_LIBIDN=ON \
  -DWITH_SSL=ON \
  -DWITH_CUPS=ON \
  -DWITH_LUA=OFF \
  -DWITH_TIFF=ON \
%{?!with_jasper:-DWITH_JASPER=OFF} \
%{?!with_openexr:-DWITH_OPENEXR=OFF} \
  -DWITH_UTEMPTER=ON \
%{?!with_avahi:-DWITH_AVAHI=OFF} \
%{?!with_elficon:-DWITH_ELFICON=OFF} \
%{?!with_pcre:-DWITH_PCRE=OFF} \
%{?!with_pcre2:-DWITH_PCRE2=OFF} \
%{?!with_inotify:-DWITH_INOTIFY=OFF} \
%{?!with_gamin:-DWITH_GAMIN=OFF} \
%{?!with_tdehwlib:-DWITH_TDEHWLIB=OFF} \
%{?!with_tdehwlib:-DWITH_TDEHWLIB_DAEMONS=OFF} \
%{?with_hal:-DWITH_HAL=ON} \
%{?with_devkitpower:-DWITH_DEVKITPOWER=ON} \
%{?with_systemd:-DWITH_LOGINDPOWER=ON} \
%{?!with_upower:-DWITH_UPOWER=OFF} \
%{?!with_udisks:-DWITH_UDISKS=OFF} \
%{?!with_udisks2:-DWITH_UDISKS2=OFF} \
  -DWITH_UDEVIL=OFF \
  -DWITH_CONSOLEKIT=ON \
%{?with_nm:-DWITH_NETWORK_MANAGER_BACKEND=ON} \
  -DWITH_SUDO_TDESU_BACKEND=OFF \
  -DWITH_OLD_XDG_STD=OFF \
  -DWITH_PCSC=ON \
  -DWITH_PKCS=ON \
  -DWITH_CRYPTSETUP=ON \
%{?!with_lzma:-DWITH_LZMA=OFF} \
  -DWITH_LIBBFD=OFF \
%{?!with_xrandr:-DWITH_XRANDR=OFF} \
  -DWITH_XCOMPOSITE=ON \
  -DWITH_KDE4_MENU_SUFFIX=OFF \
  \
  -DWITH_ASPELL=ON \
%{?!with_hspell:-DWITH_HSPELL=OFF} \
  -DWITH_TDEICONLOADER_DEBUG=OFF \
  -DUTEMPTER_HELPER=/usr/lib/utempter/utempter \
  -DWITH_IN_TREE_LIBLTDL=ON \
"-DCMAKE_INCLUDE_PATH=/usr/share/libtool-2.4/libltdl;/usr/include/dbus-1.0/dbus" \
  -DPCRE2_CODE_UNIT_WIDTH=8 \
..

%make_build VERBOSE=1 || make VERBOSE=1

%install
rm -rf "%{?buildroot}"
%make_install install DESTDIR="%{?buildroot}" -C build

# Use system-wide CA certificates
%if "%{?cacert}" != ""
rm -f "%{?buildroot}%tde_datadir/apps/kssl/ca-bundle.crt"
ln -s "%cacert" "%{?buildroot}%tde_datadir/apps/kssl/ca-bundle.crt"
%endif

# Symlinks duplicate files (mostly under 'ksgmltools2')
fdupes -s "%{?buildroot}"

# Remove setuid bit on some binaries.
chmod 0755 "%{?buildroot}%tde_bindir/kgrantpty"
chmod 0755 "%{?buildroot}%tde_bindir/kpac_dhcp_helper"
chmod 0755 "%{?buildroot}%tde_bindir/start_tdeinit"

# fileshareset 2.0 is provided separately.
# Remove integrated fileshareset 1.0 .
rm -f "%{?buildroot}%tde_bindir/filesharelist"
rm -f "%{?buildroot}%tde_bindir/fileshareset"

%changelog
* Mon Jan 27 2025 Petr Akhlamov <ahlamovpm@basealt.ru> 14.1.2-alt1_1
- converted for ALT Linux by srpmconvert tools

