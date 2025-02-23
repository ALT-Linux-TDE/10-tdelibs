METASOURCES = AUTO

noinst_HEADERS = fileprops.h
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeio/tdeio $(all_includes)

bin_PROGRAMS = tdefile
tdefile_SOURCES = fileprops.cpp
tdefile_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdefile_LDADD = $(LIB_TDEIO)

