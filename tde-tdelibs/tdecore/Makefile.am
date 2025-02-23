#    This file is part of the KDE libraries
#
#    $Id$
#
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#			  (C) 1997 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

INCLUDES = -I$(top_srcdir)/tdefx -I$(top_srcdir)/tdecore/network $(all_includes) $(LIBART_CFLAGS)

if include_SVGICONS
SVGICONS=svgicons
SVGICON_LIB=svgicons/libkdesvgicons.la
endif

SUBDIRS = malloc network $(SVGICONS) . tdeconfig_compiler tests

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

lib_LTLIBRARIES = libtdefakes.la libtdecore.la
lib_LIBRARIES = libtdefakes_nonpic.a libtdefakes_pic.a

include_HEADERS = tdeconfig.h tdeconfigskeleton.h \
	tdeconfigdata.h ksimpleconfig.h tdeconfigdialogmanager.h \
	tdeconfigbase.h kdesktopfile.h kurl.h ksock.h tdeaboutdata.h \
	tdecmdlineargs.h tdeconfigbackend.h kapp.h tdeapplication.h kuniqueapp.h \
	kuniqueapplication.h kcharsets.h tdeversion.h kpty.h kprocess.h \
	kprocctrl.h tdelocale.h kicontheme.h kiconloader.h kdebug.h \
	twinmodule.h twin.h krootprop.h tdeshortcut.h kkeynative.h tdeaccel.h \
	kglobalaccel.h tdestdaccel.h tdeshortcutlist.h kcatalogue.h \
	kregexp.h kcompletion.h kstringhandler.h \
	kstddirs.h kstandarddirs.h tdeglobal.h tdeglobalsettings.h ksharedptr.h \
	kallocator.h kvmallocator.h kcrash.h krfcdate.h \
	kinstance.h kpalette.h kipc.h klibloader.h tdetempfile.h ksavefile.h \
	krandomsequence.h knotifyclient.h kiconeffect.h \
	kaudioplayer.h kdcoppropertyproxy.h netwm.h tdeaccelmanager.h \
	netwm_def.h kpixmapprovider.h kunload.h kstaticdeleter.h \
	kextsock.h kextendedsocket.h ksockaddr.h kprocio.h kasyncio.h \
	kbufferedio.h kurldrag.h kmimesourcefactory.h kmdcodec.h ksocks.h \
	tdesycoca.h tdesycocaentry.h tdesycocatype.h kxmessages.h tdestartupinfo.h \
	klargefile.h tdemultipledrag.h kgenericfactory.h kgenericfactory.tcc \
	ktypelist.h ksortablevaluelist.h kdebugclasses.h kclipboard.h \
	kcalendarsystem.h kcalendarsystemfactory.h kmacroexpander.h \
	kmanagerselection.h kmountpoint.h kuser.h klockfile.h \
	kidna.h ktempdir.h kshell.h fixx11h.h kxerrorhandler.h tdelibs_export.h \
	tdemacros.h kde_file.h ktimezones.h

libtdefakes_la_SOURCES = fakes.c vsnprintf.c
libtdefakes_la_LDFLAGS = -version-info 6:0:2
libtdefakes_la_LIBADD  = $(LIBSOCKET)

libtdefakes_nonpic_a_SOURCES = fakes_nonpic.c vsnprintf_nonpic.c

fakes_nonpic.c:$(srcdir)/fakes.c
	-rm -f fakes_nonpic.c
	$(LN_S) $(srcdir)/fakes.c fakes_nonpic.c

vsnprintf_nonpic.c:  $(srcdir)/vsnprintf.c
	-rm -f vsnprintf_nonpic.c
	$(LN_S) $(srcdir)/vsnprintf.c vsnprintf_nonpic.c

libtdefakes_pic.a: libtdefakes.la
	-rm -f libtdefakes_pic.a fakes_pic.o vsnprintf_pic.o
	cp -f .libs/fakes.o fakes_pic.o; cp -f .libs/vsnprintf.o vsnprintf_pic.o
	ar cru libtdefakes_pic.a fakes_pic.o vsnprintf_pic.o
	ranlib libtdefakes_pic.a

noinst_HEADERS = tdeaccelaction.h tdeaccelbase.h tdeaccelprivate.h kckey.h \
	kcompletion_private.h netwm_p.h \
	kglobalaccel_x11.h kglobalaccel_win.h kkeyserver_x11.h kkeyserver.h \
	kregpriv.h tdeshortcutmenu.h tdesycocadict.h tdesycocafactory.h netsupp.h \
	kcheckaccelerators.h kcalendarsystemgregorian.h \
	kcalendarsystemhijri.h kcalendarsystemhebrew.h kcalendarsystemjalali.h \
	kprotocolinfofactory.h kqiodevicegzip_p.h kiconloader_p.h

libtdecore_la_SOURCES = libintl.cpp tdeapplication.cpp \
	kdebug.cpp netwm.cpp tdeconfigbase.cpp tdeconfig.cpp  ksimpleconfig.cpp \
	tdeconfigbackend.cpp kmanagerselection.cpp kdesktopfile.cpp \
	kstandarddirs.cpp ksock.cpp kpty.cpp kprocess.cpp kprocctrl.cpp \
	tdelocale.cpp krfcdate.cpp kiconeffect.cpp kicontheme.cpp \
	kiconloader.cpp twin.cpp twinmodule.cpp krootprop.cpp kcharsets.cpp \
	kckey.cpp tdeshortcut.cpp kkeynative_x11.cpp kkeyserver_x11.cpp \
	tdeaccelaction.cpp tdeshortcutmenu.cpp tdeaccelbase.cpp tdeaccel.cpp \
	kglobalaccel_x11.cpp kglobalaccel.cpp tdestdaccel.cpp tdeshortcutlist.cpp \
	kcrash.cpp kurl.cpp kregexp.cpp tdeglobal.cpp tdeglobalsettings.cpp \
	kallocator.cpp kvmallocator.cpp kmimesourcefactory.cpp \
	kinstance.cpp kpalette.cpp kipc.cpp klibloader.cpp tdetempfile.cpp \
	kuniqueapplication.cpp tdeaccelmanager.cpp \
	ksavefile.cpp krandomsequence.cpp kstringhandler.cpp kcompletion.cpp \
	tdecmdlineargs.cpp tdeaboutdata.cpp kcompletionbase.cpp knotifyclient.cpp \
	kaudioplayer.cpp kdcoppropertyproxy.cpp \
	ksockaddr.cpp kextsock.cpp netsupp.cpp kprocio.cpp kbufferedio.cpp \
	kpixmapprovider.cpp kurldrag.cpp \
	kmdcodec.cpp ksocks.cpp fakes.c vsnprintf.c \
	tdesycoca.cpp tdesycocadict.cpp tdesycocafactory.cpp tdesycoca.skel \
	kxmessages.cpp tdestartupinfo.cpp kcatalogue.cpp kasyncio.cpp \
	tdemultipledrag.cpp kstaticdeleter.cpp kappdcopiface.cpp \
	kappdcopiface.skel kclipboard.cpp kcheckaccelerators.cpp \
	tdeversion.cpp kdebugdcopiface.cpp kdebugdcopiface.skel \
	kcalendarsystem.cpp kcalendarsystemgregorian.cpp \
	kcalendarsystemhijri.cpp kcalendarsystemhebrew.cpp \
	kcalendarsystemfactory.cpp kmacroexpander.cpp kidna.cpp \
	ktempdir.cpp kshell.cpp kmountpoint.cpp kcalendarsystemjalali.cpp \
	kprotocolinfo_tdecore.cpp kprotocolinfofactory.cpp kxerrorhandler.cpp \
	kuser.cpp tdeconfigskeleton.cpp tdeconfigdialogmanager.cpp klockfile.cpp \
	kqiodevicegzip_p.cpp ktimezones.cpp

libtdecore_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_MT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 6:0:2 -no-undefined
libtdecore_la_LIBADD = malloc/libklmalloc.la network/libtdecorenetwork.la $(SVGICON_LIB) ../dcop/libDCOP.la ../libltdl/libltdlc.la \
	$(LIBART_LIBS) $(LIB_IDN) $(top_builddir)/tdefx/libtdefx.la $(LIB_TQT) $(LIBSM) $(LIB_X11) $(LIBZ) $(LIBDL)
libtdecore_la_NMCHECK = $(srcdir)/libtdecore.nmcheck
libtdecore_la_NMCHECKWEAK = $(srcdir)/libtdecore_weak.nmcheck $(srcdir)/libtqt-mt_weak.nmcheck \
	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/tdecore/standard_weak.nmcheck

libtdecore_la_METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/tdelibs/tdecore

kdebugdir = $(kde_confdir)
kdebug_DATA = kdebug.areas kdebugrc language.codes

.PHONY: kckey_h parser
kckey_h : $(srcdir)/generate_keys.sh $(QNAMESPACE_H)
	(cd $(srcdir) && ./generate_keys.sh $(QNAMESPACE_H))

# Also uninstall the old names of the KDE color palette files
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_locale)
	$(INSTALL_DATA) $(srcdir)/all_languages.desktop $(DESTDIR)$(kde_locale)/all_languages
	$(mkinstalldirs) $(DESTDIR)$(includedir)/tdeio
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/knotify
	$(INSTALL_DATA) $(srcdir)/eventsrc $(DESTDIR)$(kde_datadir)/knotify/eventsrc
	$(INSTALL_DATA) $(srcdir)/kmdcodec_compat.h $(DESTDIR)$(includedir)/tdeio/kmdcodec.h
	rm -f "$(DESTDIR)$(kde_confdir)/colors/40 Colors"
	rm -f $(DESTDIR)$(kde_confdir)/colors/Web
	rm -f $(DESTDIR)$(kde_confdir)/colors/Royal

uninstall-local:
	-rm -f $(DESTDIR)$(includedir)/tdeio/kmdcodec.h

# If you add a color palette file here, please change tdelibs/tdeui/kcolordialog.cpp too to allow to translate the name
colors_DATA = 40.colors Web.colors Royal.colors Rainbow.colors
colorsdir = $(kde_confdir)/colors

EXTRA_DIST = generate_keys.sh $(kdebug_DATA) \
	DESIGN.COLOR KCONFIG_DESIGN eventsrc \
	kentities.c kentities.gperf makeentities kmdcodec_compat.h \
	all_languages.desktop

kcharsets.lo: kentities.c

parser: kentities.gperf
	cd $(srcdir) && gperf -a -L "ANSI-C" -E -C -c -o -t -k '*' -Nkde_findEntity -D -Hhash_Entity -Wwordlist_Entity -s 2 kentities.gperf > kentities.c

bin_PROGRAMS = tde-config kgrantpty

kde_config_SOURCES = tde-config.cpp
kde_config_LDADD = ./libtdecore.la
kde_config_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS)

kgrantpty_SOURCES = kgrantpty.c
kgrantpty_CFLAGS= $(KDE_USE_FPIE)
kgrantpty_LDFLAGS = $(KDE_USE_PIE) $(KDE_RPATH) $(all_libraries)

# kgrantpty needs to be installed setuid root
install-exec-hook:
	@(chown 0 $(DESTDIR)$(bindir)/kgrantpty && chmod 4755 $(DESTDIR)$(bindir)/kgrantpty) || echo "Please make kgrantpty setuid root" >&2
	@echo ""
	@echo "kgrantpty is by default installed with a set SETUID root bit!"
	@echo "This is needed for konsole, etc. to ensure that they can't be eavesdropped."
	@echo ""

DISTCLEANFILES = tde-config.cpp

DOXYGEN_REFERENCES = dcop tdeui tdeparts tdeio
DOXYGEN_EXCLUDE = malloc kde_file.h
DOXYGEN_SET_WARN_IF_UNDOCUMENTED = YES

include ../admin/Doxyfile.am
