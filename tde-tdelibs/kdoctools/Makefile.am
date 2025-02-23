
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeioslave -I$(top_srcdir)/tdeioslave/bzip2 $(all_includes) $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)
AM_LDFLAGS = $(KDE_RPATH) $(LIBXML_RPATH) $(LIBXSLT_RPATH) $(all_libraries)

noinst_LTLIBRARIES = libkbzipfilter_dummy.la

libkbzipfilter_dummy_la_SOURCES = kbzip2filter.cpp

####### Files

kde_module_LTLIBRARIES = tdeio_help.la tdeio_ghelp.la

tdeio_help_la_SOURCES = tdeio_help.cpp main.cpp xslt.cpp
tdeio_help_la_LIBADD = $(LIBXSLT_LIBS) $(LIBXML_LIBS)  libkbzipfilter_dummy.la $(LIB_TDEIO) $(LIBBZ2) $(LIB_TQT) $(LIB_TDECORE)
tdeio_help_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

tdeio_ghelp_la_SOURCES = tdeio_help.cpp main_ghelp.cpp xslt.cpp
tdeio_ghelp_la_LIBADD = $(LIBXSLT_LIBS) $(LIBXML_LIBS) libkbzipfilter_dummy.la $(LIB_TDEIO) $(LIBBZ2) $(LIB_TQT) $(LIB_TDECORE)
tdeio_ghelp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = tdeio_help.h

kdelnk_DATA = help.protocol ghelp.protocol
kdelnkdir = $(kde_servicesdir)

bin_PROGRAMS = meinproc 
meinproc_LDADD = xslt_pure.o libkbzipfilter_dummy.la $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(LIB_TDEIO) $(LIBBZ2) $(LIB_TDECORE)
meinproc_SOURCES = meinproc.cpp

xslt_pure.o: xslt.cpp
	-rm -f xslt_pure.cpp
	$(LN_S) $(srcdir)/xslt.cpp xslt_pure.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) -DSIMPLE_XSLT $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) $(CXXFLAGS) -c xslt_pure.cpp

bin_SCRIPTS = checkXML

METASOURCES = AUTO

update_DATA = tdeio_help.upd
update_SCRIPTS = move_tdeio_help_cache.sh
updatedir = $(kde_datadir)/tdeconf_update

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdeio_help.pot

install-data-local:
	@(for subdir in customization docbook; do \
	   for dir in `( cd $(srcdir)/$$subdir && find . -type d ! -name CVS | grep -v '\.svn' )`; do \
		$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/ksgmltools2/$$subdir/$$dir ;\
		for file in `(cd $(srcdir)/$$subdir && ls -1 $$dir/*)`; do \
		    if test -f $(srcdir)/$$subdir/$$file; then \
			$(INSTALL_DATA) $(srcdir)/$$subdir/$$file  $(DESTDIR)$(kde_datadir)/ksgmltools2/$$subdir/$$file; fi ;\
	   	done ;\
	   done ;\
	done )

uninstall-local:
	-rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/customization	
	-rm -rf $(DESTDIR)$(kde_datadir)/ksgmltools2/docbook

kbzip2filter.cpp:
	-$(LN_S) $(srcdir)/../tdeioslave/bzip2/kbzip2filter.cpp .

CLEANFILES = kbzip2filter.cpp xslt_pure.o xslt_pure.cpp checkXML


include $(top_srcdir)/admin/Doxyfile.am

