INCLUDES = $(all_includes)

noinst_HEADERS = highcontrastconfig.h
kde_module_LTLIBRARIES = tdestyle_highcontrast_config.la
tdestyle_highcontrast_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
tdestyle_highcontrast_config_la_LIBADD  = $(LIB_TQT) $(LIB_TDECORE)
tdestyle_highcontrast_config_la_SOURCES = highcontrastconfig.cpp
tdestyle_highcontrast_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/tdestyle_highcontrast_config.pot
