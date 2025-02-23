lib_LTLIBRARIES = libtdescreensaver.la
libtdescreensaver_la_SOURCES = main.cpp tdescreensaver.cpp
libtdescreensaver_la_LIBADD  = $(LIB_TQT) $(LIB_TDECORE) $(LIB_X11)
if undefined_symbols_allowed
libtdescreensaver_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 6:0:2
else
libtdescreensaver_la_LDFLAGS = $(all_libraries) -static
endif 

include_HEADERS = tdescreensaver.h tdescreensaver_vroot.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/libtdescreensaver.pot 

include $(top_srcdir)/admin/Doxyfile.am
