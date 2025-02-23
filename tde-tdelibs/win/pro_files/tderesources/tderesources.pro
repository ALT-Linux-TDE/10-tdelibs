TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDERESOURCES_LIB

INCLUDEPATH	+= $(KDELIBS)/ab

LIBS += $$KDELIBDESTDIR\tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeio$$KDELIB_SUFFIX 

system( bash kmoc )
system( bash kdcopidl )

TARGET = tderesources$$KDEBUG

SOURCES = \
configdialog.cpp \
configpage.cpp \
configwidget.cpp \
factory.cpp \
kcmtderesources.cpp \
managerimpl.cpp \
resource.cpp \
selectdialog.cpp \
testresources.cpp

generated:
SOURCES += \
manageriface_skel.cpp \
manageriface_stub.cpp

HEADERS		=
