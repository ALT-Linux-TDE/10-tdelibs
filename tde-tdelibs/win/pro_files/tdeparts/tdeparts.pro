TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDEPARTS_LIB

TARGET		= tdeparts$$KDEBUG

LIBS += $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
 $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeio$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/tdeio/tdefile

system( bash kmoc )

SOURCES = \
browserextension.cpp \
browserinterface.cpp \
browserrun.cpp \
dockmainwindow.cpp \
event.cpp \
factory.cpp \
historyprovider.cpp \
mainwindow.cpp \
part.cpp \
partmanager.cpp \
plugin.cpp \
statusbarextension.cpp

