TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDEPRINT_LIB

TARGET		= tdeprint$$KDEBUG

SOURCES = \
kprinter.cpp \
kpreloadobject.cpp

HEADERS =
