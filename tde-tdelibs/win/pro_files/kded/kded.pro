TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

LIBS += $$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeio$$KDELIB_SUFFIX

# icon
LIBS+=$(KDELIBS)/win/resources/tdebuildsycoca.res

INCLUDEPATH += $(KDELIBS)/kded

system( bash kmoc )

!contains(CONFIG,GUI) {
	!contains(KW_CONFIG,release) {
		TARGET = tdebuildsycoca_d
	}
	contains(KW_CONFIG,release) {
		TARGET = tdebuildsycoca
	}
}

SOURCES = \
tdebuildsycoca.cpp \
kbuildimageiofactory.cpp \
kbuildprotocolinfofactory.cpp \
kbuildservicefactory.cpp \
kbuildservicegroupfactory.cpp \
kbuildservicetypefactory.cpp \
kctimefactory.cpp \
vfolder_menu.cpp

HEADERS		=

TRANSLATIONS    = tdebuildsycoca_pl.ts

