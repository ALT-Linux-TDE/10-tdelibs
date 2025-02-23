INCLUDES    = -I$(top_srcdir)/interfaces $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = tdetexteditor_isearch.la

tdetexteditor_isearch_la_SOURCES = ISearchPlugin.cpp
tdetexteditor_isearch_la_LIBADD = $(top_builddir)/interfaces/tdetexteditor/libtdetexteditor.la $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEUI)
tdetexteditor_isearch_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
          
isearchdatadir = $(kde_datadir)/tdetexteditor_isearch
isearchdata_DATA = tdetexteditor_isearchui.rc

kde_services_DATA = tdetexteditor_isearch.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdetexteditor_isearch.pot

