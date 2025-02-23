TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )


TARGET = maketdewidgets
DESTDIR = .

win32 {
#CONFIG -= console
#CONFIG += windows
}

SOURCES = maketdewidgets.cpp

HEADERS = 



