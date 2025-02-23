INCLUDES=$(all_includes)

lib_LTLIBRARIES = libtdentlm.la
METASOURCES     = AUTO

tdentlmincludedir = $(includedir)/tdeio
tdentlminclude_HEADERS = tdentlm.h

libtdentlm_la_SOURCES = tdentlm.cpp des.cpp
libtdentlm_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined
libtdentlm_la_LIBADD = $(LIB_TDECORE) $(LIB_TQT)

