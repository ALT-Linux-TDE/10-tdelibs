SUBDIRS = ksettings .
INCLUDES = -I$(top_srcdir)/interfaces/kregexpeditor $(all_includes)

lib_LTLIBRARIES   = libtdeutils.la
libtdeutils_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
# Needs TDETrader
libtdeutils_la_LIBADD  = ../tdeio/libtdeio.la ksettings/libksettings.la $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDEUI) $(LIB_TDECORE) $(LIB_X11)

libtdeutils_la_SOURCES = kfind.cpp kfinddialog.cpp kreplace.cpp \
                       kreplacedialog.cpp tdecmoduleinfo.cpp tdecmoduleloader.cpp \
                       kcmultidialog.cpp tdemultitabbar.cpp kplugininfo.cpp \
                       tdecmoduleproxy.cpp kpluginselector.cpp \
                       tdecmodulecontainer.cpp tdecmoduleproxyIface.skel \
                       tdecmoduleproxyIfaceImpl.cpp

include_HEADERS = kfind.h kfinddialog.h kreplace.h kreplacedialog.h \
                  tdecmoduleinfo.h tdecmoduleloader.h kcmultidialog.h \
                  tdemultitabbar.h kplugininfo.h tdecmoduleproxy.h \
                  kpluginselector.h tdecmodulecontainer.h

noinst_HEADERS = tdemultitabbar_p.h kpluginselector_p.h \
                 tdecmoduleproxyIface.h tdecmoduleproxyIfaceImpl.h

kde_servicetypes_DATA = kplugininfo.desktop

METASOURCES = AUTO

DOXYGEN_REFERENCES = tdecore tdeui tdeio
include ../admin/Doxyfile.am
