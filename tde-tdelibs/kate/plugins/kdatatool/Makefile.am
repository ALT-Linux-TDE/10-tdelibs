INCLUDES    = -I$(top_srcdir)/interfaces -I$(top_srcdir)/tdefx $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = tdetexteditor_kdatatool.la

tdetexteditor_kdatatool_la_SOURCES = kate_kdatatool.cpp
tdetexteditor_kdatatool_la_LIBADD = $(top_builddir)/interfaces/tdetexteditor/libtdetexteditor.la $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEFILE) $(LIB_TDEUI)
tdetexteditor_kdatatool_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kdatatooldatadir = $(kde_datadir)/tdetexteditor_kdatatool
kdatatooldata_DATA = tdetexteditor_kdatatoolui.rc

kde_services_DATA = tdetexteditor_kdatatool.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdetexteditor_kdatatool.pot


