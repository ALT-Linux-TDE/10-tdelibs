TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX

LIBS -= $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdefx$$KDELIB_SUFFIX

TARGET		= dcopidl

DEFINES += YY_ALWAYS_INTERACTIVE

SOURCES = \
main.cpp \
scanner.cpp \
yacc.cpp

HEADERS		=
