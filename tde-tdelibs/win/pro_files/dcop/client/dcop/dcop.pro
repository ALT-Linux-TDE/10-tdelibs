TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
#DEFINES += MAKE_DCOP_LIB

DEFINES += TDECORE_EXPORT=

TARGET		= dcop

#system( bash kmoc .. )

#INCLUDEPATH += .. ../moc

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX

SOURCES	+= ../dcop.cpp
