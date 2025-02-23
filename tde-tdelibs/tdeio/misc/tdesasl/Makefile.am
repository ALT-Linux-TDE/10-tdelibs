INCLUDES=$(all_includes)

lib_LTLIBRARIES = libtdesasl.la
METASOURCES     = AUTO

tdesaslincludedir = $(includedir)/tdeio
tdesaslinclude_HEADERS = tdesasl.h

libtdesasl_la_SOURCES = tdesasl.cpp
libtdesasl_la_LDFLAGS = $(all_libraries) -version-info 3:0:2 -no-undefined
libtdesasl_la_LIBADD = $(LIB_TDECORE) $(LIB_TQT)

