INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourceevo.h dbwrapper.h

kde_module_LTLIBRARIES = tdeabc_evo.la

tdeabc_evo_la_SOURCES = dbwrapper.cpp resourceevo.cpp

tdeabc_evo_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
tdeabc_evo_la_LIBADD	= ../../../tdeabc/libtdeabc.la ../../../tdeui/libtdeui.la -ldb ../../../tdeabc/vcardparser/libvcards.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_evolution.pot

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = evolution.desktop
