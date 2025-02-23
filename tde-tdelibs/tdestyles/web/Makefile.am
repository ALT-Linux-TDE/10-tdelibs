INCLUDES              = $(all_includes)
lib_LTLIBRARIES       = webstyle.la
webstyle_la_LDFLAGS   = $(all_libraries) -module -avoid-version -no-undefined 
webstyle_la_LIBADD    = $(LIB_TDEUI)
webstyle_la_SOURCES   = webstyle.cpp plugin.cpp
noinst_HEADERS        = webstyle.h 

tdestylewebdata_DATA = web.themerc
tdestylewebdatadir = $(kde_datadir)/tdestyle/themes
EXTRA_DIST = $(tdestylewebdata_DATA)
