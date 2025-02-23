TEMPLATE        = app
#CONFIG		= qt warn_on thread

include( $(KDELIBS)/win/common.pro )


CONFIG -= release
CONFIG -= windows
CONFIG += debug
CONFIG += console

unix:DEFINES   = NO_INCLUDE_MOCFILES
unix:LIBS       += -ltdemdi -L$(TDEDIR)/lib -ltdecore -ltdeui -lDCOP -ltdeparts
unix:INCLUDEPATH     += . .. $(TDEDIR)/include

LIBS += $$KDELIBDESTDIR\tdemdi$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeparts$$KDELIB_SUFFIX

HEADERS = hello.h \
          mainwidget.h

SOURCES = hello.cpp \
          mainwidget.cpp \
          main.cpp

TARGET    = kfourchildren
