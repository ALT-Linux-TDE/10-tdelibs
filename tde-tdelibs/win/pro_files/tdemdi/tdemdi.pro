TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_KMDI_LIB

TARGET		= tdemdi$$KDEBUG

LIBS += $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeutils$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeparts$$KDELIB_SUFFIX

INCLUDEPATH += res $(KDELIBS)/tdeutils $(KDELIBS)/tdeui

system( bash kmoc )

SOURCES = \
tdemdichildarea.cpp \
tdemdichildfrm.cpp \
tdemdichildfrmcaption.cpp \
tdemdichildview.cpp \
tdemdidockcontainer.cpp \
tdemdidocumentviewtabwidget.cpp \
tdemdifocuslist.cpp \
tdemdiguiclient.cpp \
tdemdimainfrm.cpp \
tdemditaskbar.cpp \
tdemditoolviewaccessor.cpp

HEADERS		=
