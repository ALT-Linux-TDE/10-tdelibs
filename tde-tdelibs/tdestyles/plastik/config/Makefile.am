INCLUDES = $(all_includes)

noinst_HEADERS = plastikconf.h
kde_module_LTLIBRARIES = tdestyle_plastik_config.la
tdestyle_plastik_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
tdestyle_plastik_config_la_LIBADD  = $(LIB_TDEUI) $(LIB_TQT) $(LIB_TDECORE)
tdestyle_plastik_config_la_SOURCES = plastikconf.cpp
tdestyle_plastik_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdestyle_plastik_config.pot
