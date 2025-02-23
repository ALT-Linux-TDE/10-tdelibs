SUBDIRS = tdemdi . test

INCLUDES = -I$(top_srcdir)/tdemdi/res -I$(top_srcdir)/tdeutils -I$(top_srcdir) -I$(top_srcdir)/tdemdi $(all_includes)

lib_LTLIBRARIES = libtdemdi.la
libtdemdi_la_LDFLAGS = $(all_libraries) -version-info 1:0
libtdemdi_la_LIBADD  = $(LIB_TDEPARTS) $(top_builddir)/tdeutils/libtdeutils.la $(LIB_TDEUI) $(LIB_TQT) $(LIB_TDECORE) $(LIB_X11)

libtdemdi_la_SOURCES = tdemdichildarea.cpp tdemdichildfrm.cpp tdemdichildfrmcaption.cpp \
                     tdemdichildview.cpp tdemdimainfrm.cpp tdemditaskbar.cpp tdemdidockcontainer.cpp \
		     tdemditoolviewaccessor.cpp tdemdiguiclient.cpp tdemdidocumentviewtabwidget.cpp \
                     tdemdifocuslist.cpp

include_HEADERS = tdemdichildarea.h tdemdichildfrm.h tdemdichildfrmcaption.h tdemdidefines.h \
                  tdemdiiterator.h tdemdilistiterator.h tdemdimainfrm.h tdemdinulliterator.h \
                  tdemditaskbar.h tdemdichildview.h tdemditoolviewaccessor.h

METASOURCES = AUTO

include ../admin/Doxyfile.am
