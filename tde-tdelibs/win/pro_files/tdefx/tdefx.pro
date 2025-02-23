TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDEFX_LIB

TARGET		= tdefx$$KDEBUG

system( bash kmoc )

SOURCES = \
kimageeffect.cpp \
kpixmapeffect.cpp \
kpixmapsplitter.cpp \
kpixmap.cpp \
tdestyle.cpp \
kdrawutil.cpp \
kcpuinfo.cpp

HEADERS		=
