INCLUDES = -I$(top_srcdir)/tdeio -I$(top_srcdir) -I$(top_srcdir)/interfaces -I$(top_builddir)/interfaces  -I$(top_srcdir)/tdeabc -I$(top_builddir)/tdeabc $(all_includes)

lib_LTLIBRARIES = libtdetexteditor.la

libtdetexteditor_la_SOURCES = tdetexteditor.cpp \
    editinterface.cpp clipboardinterface.cpp  selectioninterface.cpp searchinterface.cpp \
    codecompletioninterface.cpp wordwrapinterface.cpp blockselectioninterface.cpp \
    configinterface.cpp cursorinterface.cpp  dynwordwrapinterface.cpp \
    printinterface.cpp highlightinginterface.cpp markinterface.cpp \
    popupmenuinterface.cpp undointerface.cpp viewcursorinterface.cpp \
    editdcopinterface.cpp editdcopinterface.skel clipboarddcopinterface.cpp \
    clipboarddcopinterface.skel selectiondcopinterface.cpp selectiondcopinterface.skel  \
    searchdcopinterface.cpp searchdcopinterface.skel markinterfaceextension.cpp \
    configinterfaceextension.cpp encodinginterface.cpp sessionconfiginterface.cpp \
    viewstatusmsginterface.cpp editorchooser_ui.ui editorchooser.cpp \
    blockselectiondcopinterface.cpp documentinfo.cpp documentdcopinfo.cpp\
    blockselectiondcopinterface.skel documentdcopinfo.skel encodingdcopinterface.cpp \
    encodingdcopinterface.skel printdcopinterface.cpp printdcopinterface.skel \
    undodcopinterface.cpp undodcopinterface.skel viewcursordcopinterface.cpp \
    viewcursordcopinterface.skel viewstatusmsgdcopinterface.cpp \
    viewstatusmsgdcopinterface.skel selectioninterfaceext.cpp selectionextdcopinterface.skel \
    texthintinterface.cpp editinterfaceext.cpp variableinterface.cpp templateinterface.cpp


libtdetexteditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
libtdetexteditor_la_LIBADD = $(LIB_TDEPARTS) $(top_builddir)/dcop/libDCOP.la $(LIB_TDEABC) $(LIB_TDECORE) $(LIB_TQT) $(LIB_TDEFILE) $(LIB_TDEUI)

tdetexteditorinclude_HEADERS = document.h view.h editor.h plugin.h editinterface.h undointerface.h \
                             selectioninterface.h cursorinterface.h \
                             clipboardinterface.h popupmenuinterface.h \
                             viewcursorinterface.h searchinterface.h highlightinginterface.h \
                             blockselectioninterface.h codecompletioninterface.h \
                             configinterface.h markinterface.h printinterface.h wordwrapinterface.h \
                             dynwordwrapinterface.h markinterfaceextension.h configinterfaceextension.h \
                             encodinginterface.h viewstatusmsginterface.h  sessionconfiginterface.h editorchooser.h \
			     documentinfo.h selectioninterfaceext.h selectionextdcopinterface.h texthintinterface.h \
			     editinterfaceext.h variableinterface.h templateinterface.h

tdetexteditorincludedir = $(includedir)/tdetexteditor

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = tdetexteditor.desktop tdetexteditoreditor.desktop tdetexteditorplugin.desktop

kcm_tdetexteditor_DATA = kcm_tdetexteditor.desktop
kcm_tdetexteditordir = $(kde_datadir)/kcm_componentchooser

templateinterface.lo: $(top_builddir)/tdeabc/addressee.h

include ../../admin/Doxyfile.am

