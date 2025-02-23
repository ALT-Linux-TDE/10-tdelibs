TEMPLATE	= lib

CONFIG += tdestyle

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDESTYLE_LIB 

LIBS += $$KDELIBDESTDIR\tdefx$$KDELIB_SUFFIX

