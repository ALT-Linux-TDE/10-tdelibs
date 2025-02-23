include( ../common.pro )

LIBS += $$KDELIBDESTDIR\tdecore$$KDELIB_SUFFIX

TARGET		= kthemestyle$$KDEBUG

SOURCES = \
kthemestyle.cpp \
kthemebase.cpp \
tdestyledirs.cpp

system( moc kthemestyle.h -o moc/kthemestyle.moc )
system( moc kthemebase.h -o moc/kthemebase.moc )


