INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libtderesources.la

libtderesources_la_SOURCES = resource.cpp \
  factory.cpp \
  manageriface.skel \
  manageriface.stub \
  managerimpl.cpp \
  configwidget.cpp \
  configdialog.cpp \
  selectdialog.cpp \
  configpage.cpp

libtderesources_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libtderesources_la_LIBADD = $(LIB_TDEUI) $(LIB_TDEIO) $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDECORE)

kresincludedir = $(includedir)/tderesources
kresinclude_HEADERS = resource.h \
  configwidget.h \
  factory.h \
  manageriface.h \
  managerimpl.h \
  manager.h \
  selectdialog.h \
  configdialog.h \
  configpage.h

kde_module_LTLIBRARIES = kcm_tderesources.la

kcm_tderesources_la_SOURCES = kcmtderesources.cpp
kcm_tderesources_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_tderesources_la_LIBADD = libtderesources.la $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDECORE)


check_PROGRAMS = testresources

testresources_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testresources_LDADD   = libtderesources.la
testresources_SOURCES = testresources.cpp

xdg_apps_DATA = tderesources.desktop

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = tderesources_plugin.desktop tderesources_manager.desktop

include ../admin/Doxyfile.am
