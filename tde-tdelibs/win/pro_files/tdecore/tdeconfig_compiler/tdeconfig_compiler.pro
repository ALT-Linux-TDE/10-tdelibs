TEMPLATE	= app

include( $(KDELIBS)/win/common.pro )

TARGET = tdeconfig_compiler

win32 {
#CONFIG -= console
#CONFIG += windows
}

#system( bash kmoc )

SOURCES = tdeconfig_compiler.cpp

HEADERS = 


