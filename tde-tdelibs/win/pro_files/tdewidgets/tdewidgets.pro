TEMPLATE	= lib
CONFIG += trinitylib #this is a dynamic kde library

include( $(KDELIBS)/win/common.pro )

system( maketdewidgets -o tdewidgets.cpp kde.widgets )

#no _d because it's a special case

TARGET = tdewidgets

DESTDIR		= $$KDELIBDESTDIR/trinity/plugins/designer

LIBS +=  $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeio$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeabc$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeutils$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tderesources$$KDELIB_SUFFIX 

SOURCES += classpreviews.cpp tdewidgets.cpp

