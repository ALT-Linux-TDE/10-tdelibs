TEMPLATE	= app

CONFIG += notdecore notdefx notdeui
include( $(KDELIBS)/win/common.pro )

# needed to export library classes:

LIBS += $$KDELIBDESTDIR/kdeice$$KDELIB_SUFFIX

TARGET		= iceauth

SOURCES = \
iceauth.c \
process.c

HEADERS		=
