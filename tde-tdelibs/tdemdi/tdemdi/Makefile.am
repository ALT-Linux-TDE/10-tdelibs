INCLUDES = -I$(top_srcdir)/tdeutils -I$(top_srcdir) -I$(top_srcdir)/tdemdi $(all_includes)

lib_LTLIBRARIES = libtdemdi2.la
libtdemdi2_la_LDFLAGS = $(all_libraries) -version-info 1:0
libtdemdi2_la_LIBADD  = $(LIB_TDEPARTS) $(top_builddir)/tdeutils/libtdeutils.la $(LIB_TDEUI) $(LIB_TQT) $(LIB_TDECORE)

libtdemdi2_la_SOURCES = mainwindow.cpp dockcontainer.cpp toolviewaccessor.cpp guiclient.cpp tabwidget.cpp

tdemdiincludedir = $(includedir)/tdemdi
tdemdiinclude_HEADERS = global.h mainwindow.h toolviewaccessor.h tabwidget.h

METASOURCES = AUTO

include ../../admin/Doxyfile.am
