INCLUDES    = -I$(top_srcdir)/interfaces $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = tdetexteditor_docwordcompletion.la

tdetexteditor_docwordcompletion_la_SOURCES = docwordcompletion.cpp
tdetexteditor_docwordcompletion_la_LIBADD = $(top_builddir)/interfaces/tdetexteditor/libtdetexteditor.la $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEUI)
tdetexteditor_docwordcompletion_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

docwordcompletiondatadir = $(kde_datadir)/tdetexteditor_docwordcompletion
docwordcompletiondata_DATA = docwordcompletionui.rc

kde_services_DATA = tdetexteditor_docwordcompletion.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdetexteditor_docwordcompletion.pot
