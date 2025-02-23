INCLUDES    = -I$(top_srcdir)/interfaces $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = tdetexteditor_insertfile.la

tdetexteditor_insertfile_la_SOURCES = insertfileplugin.cpp
tdetexteditor_insertfile_la_LIBADD = $(top_builddir)/interfaces/tdetexteditor/libtdetexteditor.la $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEFILE) $(LIB_TDEUI)
tdetexteditor_insertfile_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
          
insertfiledatadir = $(kde_datadir)/tdetexteditor_insertfile
insertfiledata_DATA = tdetexteditor_insertfileui.rc

kde_services_DATA = tdetexteditor_insertfile.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdetexteditor_insertfile.pot
