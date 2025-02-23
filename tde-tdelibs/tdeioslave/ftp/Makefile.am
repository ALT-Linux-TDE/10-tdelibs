INCLUDES= $(all_includes)

####### Files

kde_module_LTLIBRARIES = tdeio_ftp.la

tdeio_ftp_la_SOURCES = ftp.cpp
tdeio_ftp_la_LIBADD  = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE)
tdeio_ftp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) 

noinst_HEADERS = ftp.h

kdelnk_DATA = ftp.protocol
kdelnkdir = $(kde_servicesdir)


