TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

LIBS -= $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdefx$$KDELIB_SUFFIX

TARGET		= dcopidl2cpp

#DEFINES += YY_ALWAYS_INTERACTIVE

SOURCES = \
main.cpp \
skel.cpp \
stubimpl.cpp \
stub.cpp

HEADERS		=
