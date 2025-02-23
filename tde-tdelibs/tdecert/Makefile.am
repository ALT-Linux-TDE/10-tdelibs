INCLUDES= -I$(top_srcdir)/tdeio/kssl -I$(top_builddir)/tdeio/kssl -I$(top_srcdir) $(SSL_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = libtdecertpart.la

libtdecertpart_la_SOURCES	= tdecertpart.cpp

libtdecertpart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libtdecertpart_la_LIBADD = $(LIB_TDEPARTS) $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDEFILE) $(LIB_TDECORE)
libtdecertpart_la_DEPENDENCIES = $(LIB_TDEPARTS)

libtdecertpart_la_METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = tdecertpart.desktop

rcdir = $(kde_datadir)/tdecertpart
rc_DATA = tdecertpart.rc

