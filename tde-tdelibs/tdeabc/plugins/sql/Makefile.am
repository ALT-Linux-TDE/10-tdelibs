INCLUDES = -I$(top_srcdir)/tdeabc -I$(top_builddir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcesql.h resourcesqlconfig.h

kde_module_LTLIBRARIES = tdeabc_sql.la

tdeabc_sql_la_SOURCES = resourcesql.cpp resourcesqlconfig.cpp

tdeabc_sql_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
tdeabc_sql_la_LIBADD	= ../../libtdeabc.la ../../../tdeui/libtdeui.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_sql.pot

linkdir		= $(kde_datadir)/tderesources/contact
link_DATA	= sql.desktop
EXTRA_DIST	= $(link_DATA)
