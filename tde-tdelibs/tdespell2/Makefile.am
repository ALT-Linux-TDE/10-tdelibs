SUBDIRS = . ui tests plugins
METASOURCES = AUTO

noinst_LTLIBRARIES = libtdespell2base.la

AM_CPPFLAGS = -DKDE_NO_COMPAT -I$(top_srcdir)/tdeutils $(all_includes)

tdespell2baseincludedir = $(includedir)/tdespell2

tdespell2baseinclude_HEADERS = settings.h filter.h \
	dictionary.h broker.h \
	backgroundchecker.h defaultdictionary.h 

libtdespell2base_la_SOURCES = settings.cpp broker.cpp \
	client.cpp filter.cpp backgroundchecker.cpp \
	backgroundthread.cpp backgroundengine.cpp \
	defaultdictionary.cpp 

libtdespell2base_la_LDFLAGS = -no-undefined $(all_libraries)

servicetype_DATA = tdespellclient.desktop
servicetypedir = $(kde_servicetypesdir)

include $(top_srcdir)/admin/Doxyfile.am
