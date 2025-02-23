INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = tdeprint_tool_escputil.la
tdeprint_tool_escputil_la_SOURCES = escpwidget.cpp
tdeprint_tool_escputil_la_METASOURCES = AUTO
tdeprint_tool_escputil_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_tool_escputil_la_LIBADD = $(LIB_TDEUI) $(LIB_TQT) $(LIB_TDECORE)

noinst_HEADERS = escpwidget.h

service_DATA = escputil.desktop
servicedir = $(kde_datadir)/tdeprint/tools
