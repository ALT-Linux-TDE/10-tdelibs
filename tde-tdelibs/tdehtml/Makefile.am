#    This file is part of the KDE libraries
#    Copyright (C) 1997 Martin Jones (mjones@kde.org)
#              (C) 1997 Torben Weis (weis@kde.org)

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

KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)

if DOJAVA
JAVA_SUBDIR=java
LIBKJAVA=
endif

SUBDIRS = misc dom css xml html rendering pics $(JAVA_SUBDIR) ecma . tdemultipart

CLEANFILES = dummy.cpp

lib_LTLIBRARIES = libtdehtml.la
kde_module_LTLIBRARIES = libtdehtmlpart.la tdehtmlimagepart.la

libtdehtml_la_SOURCES = kjserrordlg.ui tdehtmlview.cpp tdehtml_part.cpp tdehtml_run.cpp\
			tdehtml_factory.cpp tdehtml_settings.cpp tdehtml_events.cpp \
			tdehtml_ext.cpp tdehtml_pagecache.cpp tdehtml_iface.skel \
			tdehtml_iface.cpp tdehtml_printsettings.cpp htmlpageinfo.ui \
			tdehtmlimage.cpp

libtdehtml_la_METASOURCES = AUTO

include_HEADERS = tdehtmlview.h tdehtml_part.h tdehtml_events.h tdehtml_settings.h tdehtmldefaults.h

noinst_HEADERS = design.h testtdehtml.h test_regression.h tdehtml_ext.h \
		 tdehtml_pagecache.h tdehtmlimage.h tdehtmlpart_p.h tdehtml_printsettings.h \
		kjserrordlg.ui.h

if include_VERSION_SCRIPT
VSCRIPT = -Wl,--version-script=$(srcdir)/libtdehtml.map
endif

libtdehtml_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(VSCRIPT) $(all_libraries)
libtdehtml_la_LIBADD = ./xml/libtdehtmlxml.la ./html/libtdehtmlhtml.la \
	./rendering/libtdehtmlrender.la ./css/libtdehtmlcss.la \
	./misc/libtdehtmlmisc.la ecma/libkjs_html.la ./dom/libtdehtmldom.la $(LIBKJAVA) \
	$(LIB_TDEPARTS) \
	$(LIBTHAI) \
	$(top_builddir)/tdeprint/libtdeprint.la  \
	$(top_builddir)/tdeutils/libtdeutils.la \
	$(top_builddir)/tdewallet/client/libtdewalletclient.la $(top_builddir)/kjs/libkjs.la $(LIB_TDEUI) $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEFILE) $(top_builddir)/dcop/libDCOP.la $(LIBJPEG) $(LIB_X11)

# init_libtdehtml is in libtdehtml.so; init_libtdehtmlpart needs to be in the part
dummy.cpp: $(srcdir)/Makefile.am
	echo '#include <tdelibs_export.h>' > $@
	echo 'extern "C" TDE_EXPORT void* init_libtdehtml();' >> $@
	echo 'extern "C" TDE_EXPORT void* init_libtdehtmlpart() { return init_libtdehtml(); } ' >> $@

libtdehtmlpart_la_SOURCES = dummy.cpp
libtdehtmlpart_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
libtdehtmlpart_la_LIBADD = libtdehtml.la $(LIB_TQT)

# init_tdehtmlimagefactory is in libtdehtml.so; init_tdehtmlimagepart needs to be in the part
imgdummy.cpp: $(srcdir)/Makefile.am
	echo '#include <tdelibs_export.h>' > $@
	echo 'extern "C"  TDE_EXPORT void* init_tdehtmlimagefactory();' >> $@
	echo 'extern "C"  TDE_EXPORT void* init_tdehtmlimagepart() { return init_tdehtmlimagefactory(); } ' >> $@

tdehtmlimagepart_la_SOURCES = imgdummy.cpp
tdehtmlimagepart_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
tdehtmlimagepart_la_LIBADD = libtdehtml.la $(LIB_TQT)

INCLUDES = -I$(top_srcdir)/kjs -I$(top_builddir)/kjs -I$(top_srcdir)/kimgio \
	-I$(srcdir)/java -I$(top_srcdir)/dcop -I$(srcdir)/misc \
	-I$(srcdir)/dom -I$(srcdir)/xml -I$(srcdir)/html -I$(srcdir)/css \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/tdeio/kssl -I$(top_builddir)/tdeio/kssl \
	-I$(top_srcdir)/tdeprint \
	-I$(top_srcdir)/interfaces \
	-I$(top_srcdir)/interfaces/kregexpeditor \
        -I$(top_srcdir)/tdewallet/client \
	-I$(top_srcdir)/tdeutils \
	-I$(top_srcdir) $(all_includes)

servicedir = $(kde_servicesdir)
service_DATA = tdehtml.desktop tdehtmlimage.desktop

rcdir = $(kde_datadir)/tdehtml
rc_DATA = tdehtml.rc tdehtml_browser.rc tdehtml_popupmenu.rc

SRCDOC_DEST=$(kde_htmldir)/en/tdelibs/tdehtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) tdecore \
	         $(include_HEADERS) -lqt

EXTRA_DIST =  CHANGES DESIGN README.HTMLWidget README.tags 

parser:
	cd $(srcdir) && \
	perl scripts/makeattrs && \
	bash scripts/maketags && \
	bash scripts/makeprop

## test program
check_PROGRAMS 	= testtdehtml
EXTRA_PROGRAMS = testtdehtml_static testregression
testtdehtml_SOURCES = testtdehtml.cpp domtreeview.cpp
testtdehtml_LDADD = libtdehtml.la 
testtdehtml_static_SOURCES = testtdehtml.cpp domtreeview.cpp 
testtdehtml_static_LDADD = libtdehtml.la
testtdehtml_static_LDFLAGS = -static
testregression_SOURCES = test_regression.cpp test_regression_fontoverload.cpp
testregression_LDADD = libtdehtml.la

DOXYGEN_REFERENCES = tdecore tdeui tdeio tdefx tdeparts
DOXYGEN_EXCLUDE =  test*.* css ecma html java tdemultipart misc pics rendering test xml

include ../admin/Doxyfile.am

.PHONY: parser

