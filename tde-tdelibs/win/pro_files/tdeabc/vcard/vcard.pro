TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_KVCARD_LIB

LIBS += $$KDELIBDESTDIR\tdecore$$KDELIB_SUFFIX 

INCLUDEPATH += $(KDELIBS)/tdeabc/vcard/include $(KDELIBS)/tdeabc/vcard/include/generated \
	$(KDELIBS)/tdeabc/vcardparser

system( bash kmoc )
system( bash kdcopidl )

TARGET		= kvcard$$KDEBUG

SOURCES = \
vCard-all.cpp

HEADERS		=

