# release version with compiled-in tdefx classes
# this is used eg. for QT-only apps like Installer

TEMPLATE	= lib

unix { 
	INCLUDEPATH += moc
	#force finding libraries in current dir (for installer, etc):
	QMAKE_LFLAGS += -Wl,-rpath,.
	DEFINES += TDEFX_EXPORT=
}


CONFIG += tdestyle

KW_CONFIG = release
CONFIG -= debug
CONFIG += release

include( $(KDELIBS)/win/common.pro )


LIBS= #DONT BE DEPENDENT ON ANY OTHER LIBS

CONFIG -= debug

# needed to export library classes:
DEFINES += MAKE_TDESTYLE_LIB MAKE_TDEFX_LIB

TARGET		= thinkeramik

SOURCES = \
colorutil.cpp \
gradients.cpp \
thinkeramik.cpp \
pixmaploader.cpp

#compile in tdefx
SOURCES += \
../../../tdefx/tdestyle.cpp \
../../../tdefx/kimageeffect.cpp \
../../../tdefx/kpixmapeffect.cpp \
../../../tdefx/kpixmap.cpp

system( moc thinkeramik.h -o moc/thinkeramik.moc )
system( moc ../../../tdefx/tdestyle.h -o moc/tdestyle.moc )


system( bash ./genemb.sh )

