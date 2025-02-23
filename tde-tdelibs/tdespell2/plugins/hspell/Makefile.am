METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/tdespell2 -I$(top_srcdir) $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kde_module_LTLIBRARIES = tdespell_hspell.la

tdespell_hspell_la_SOURCES = tdespell_hspellclient.cpp tdespell_hspelldict.cpp

tdespell_hspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
tdespell_hspell_la_LIBADD = ../../ui/libtdespell2.la -lhspell -lz $(LIB_TQT) $(LIB_TDECORE)

service_DATA = tdespell_hspell.desktop
servicedir = $(kde_servicesdir)
