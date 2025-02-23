check_PROGRAMS = kfourchildren
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/tdemdi -I.. $(all_includes)

kfourchildren_SOURCES = hello.cpp main.cpp mainwidget.cpp
kfourchildren_LDFLAGS = -no-undefined $(all_libraries)
kfourchildren_LDADD = $(top_builddir)/tdemdi/libtdemdi.la \
	$(top_builddir)/tdeutils/libtdeutils.la

