TEMPLATE	= lib

KW_CONFIG = release
CONFIG -= debug
CONFIG += release

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDEFX_LIB

system( moc tdestyle.h -o moc/tdestyle.moc )

LIBS -= "$(TDEDIR)\lib\kdewin32_$(KDE_VER).lib" "$(TDEDIR)\lib\ltdl_$(KDE_VER).lib"

SOURCES = \
kimageeffect.cpp \
kpixmapeffect.cpp \
kpixmap.cpp \
tdestyle.cpp \
kdrawutil.cpp \
kcpuinfo.cpp

HEADERS		=

TARGET		= tdefx
