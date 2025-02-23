METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/tdespell2 -I$(top_srcdir) $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kde_module_LTLIBRARIES = tdespell_ispell.la

tdespell_ispell_la_SOURCES = tdespell_ispellclient.cpp tdespell_ispelldict.cpp \
	correct.cpp		\
	good.cpp		\
	hash.cpp		\
	lookup.cpp		\
	makedent.cpp		\
	tgood.cpp		\
	ispell_checker.cpp

tdespell_ispell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
tdespell_ispell_la_LIBADD = ../../ui/libtdespell2.la  $(LIB_TQT) $(LIB_TDECORE)

service_DATA = tdespell_ispell.desktop
servicedir = $(kde_servicesdir)
