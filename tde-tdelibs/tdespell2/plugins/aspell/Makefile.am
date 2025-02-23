METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/tdespell2 -I$(top_srcdir) $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kde_module_LTLIBRARIES = tdespell_aspell.la

tdespell_aspell_la_SOURCES = tdespell_aspellclient.cpp tdespell_aspelldict.cpp

tdespell_aspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
tdespell_aspell_la_LIBADD = ../../ui/libtdespell2.la -laspell $(LIB_TQT) $(LIB_TDECORE)

service_DATA = tdespell_aspell.desktop
servicedir = $(kde_servicesdir)
