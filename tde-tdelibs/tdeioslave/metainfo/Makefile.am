## $Id$
## Makefile.am of tdebase/tdeioslave/metainfo

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
METASOURCES = AUTO

kde_module_LTLIBRARIES = tdeio_metainfo.la

tdeio_metainfo_la_SOURCES = metainfo.cpp
tdeio_metainfo_la_LIBADD = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE)
tdeio_metainfo_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = metainfo.h 

kdelnk_DATA = metainfo.protocol
kdelnkdir = $(kde_servicesdir)

#servicetypes_DATA = thumbcreator.desktop
#servicetypesdir = $(kde_servicetypesdir)

#services_DATA = imagethumbnail.desktop textthumbnail.desktop 
#                htmlthumbnail.desktop gsthumbnail.desktop
#servicesdir = $(kde_servicesdir)
