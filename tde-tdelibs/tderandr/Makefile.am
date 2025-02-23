
INCLUDES =  -I$(srcdir)/.. $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

libtderandrincludedir = $(includedir)/libtderandr
libtderandrinclude_HEADERS = randr.h lowlevel_randr.h ktimerdialog.h libtderandr.h

lib_LTLIBRARIES = libtderandr.la
libtderandr_la_SOURCES  = randr.cpp lowlevel_randr.c ktimerdialog.cpp libtderandr.cpp
METASOURCES = AUTO

libtderandr_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 0:95  -no-undefined
libtderandr_la_LIBADD = $(LIBASOUND) ../tdecore/libtdecore.la $(LIB_TQT) $(LIB_XRANDR)

DOXYGEN_REFERENCES = tdecore
include ../admin/Doxyfile.am
