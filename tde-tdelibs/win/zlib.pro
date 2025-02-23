
# adds includes and zlib library (Qt's zlib or external zlib)

exists( $(KDELIBS)/win/3rdparty/zlib/zlib.h ) {
	INCLUDEPATH += $(KDELIBS)/win/3rdparty/zlib
	LIBS += $$KDELIBDESTDIR\zdll.lib
}
!exists( $(KDELIBS)/win/3rdparty/zlib/zlib.h ) {
	INCLUDEPATH += $(TQTDIR)/src/3rdparty/zlib
}
