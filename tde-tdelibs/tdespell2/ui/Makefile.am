METASOURCES = AUTO

lib_LTLIBRARIES = libtdespell2.la
# Put most of the code in a noinst lib, for the unit tests to be able to use internal classes.
noinst_LTLIBRARIES = libtdespell2_noinst.la

KDE_CXXFLAGS = -DKDE_NO_COMPAT
INCLUDES = -I$(top_srcdir)/tdespell2 $(all_includes)

tdespell2includedir = $(includedir)/tdespell2
tdespell2include_HEADERS = dialog.h highlighter.h configdialog.h \
	configwidget.h

libtdespell2_noinst_la_SOURCES = tdespell2ui.ui configwidget.cpp \
	highlighter.cpp configui.ui configdialog.cpp

dialog.lo: tdespell2ui.h

libtdespell2_la_SOURCES = dialog.cpp
libtdespell2_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libtdespell2_la_LIBADD = libtdespell2_noinst.la ../libtdespell2base.la ../../tdeutils/libtdeutils.la  $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDECORE) $(LIB_TDEFILE)
