INCLUDES    = -I$(top_srcdir)/interfaces $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = tdetexteditor_autobookmarker.la

tdetexteditor_autobookmarker_la_SOURCES = autobookmarker.cpp
tdetexteditor_autobookmarker_la_LIBADD = $(top_builddir)/interfaces/tdetexteditor/libtdetexteditor.la
tdetexteditor_autobookmarker_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = tdetexteditor_autobookmarker.desktop

confdir=$(kde_confdir)
conf_DATA = tdetexteditor_autobookmarkerrc

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdetexteditor_autobookmarker.pot
