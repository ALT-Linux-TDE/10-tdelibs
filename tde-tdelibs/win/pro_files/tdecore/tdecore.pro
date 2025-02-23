TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )
# for kqiodevicegzip_p.cpp
include( $(KDELIBS)/win/zlib.pro )

# needed to export library classes:
DEFINES += MAKE_TDECORE_LIB

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdefx$$KDELIB_SUFFIX \
	-lqassistantclient

TARGET		= tdecore$$KDEBUG

INCLUDEPATH += $(KDELIBS)/libltdl $(KDELIBS)/tdecore/network

system( bash kmoc )
system( bash kdcopidl )

SOURCES += \
fakes.c \
kpixmapprovider.cpp \
kpalette.cpp \
kprocess.cpp \
kprocio.cpp \
kcrash.cpp \
kallocator.cpp \
knotifyclient.cpp \
kcompletionbase.cpp \
kcompletion.cpp \
kmimesourcefactory.cpp \
ksimpleconfig.cpp \
libintl.cpp \
kcatalogue.cpp \
kcalendarsystem.cpp \
kcalendarsystemfactory.cpp \
kcalendarsystemgregorian.cpp \
kcalendarsystemhebrew.cpp \
kcalendarsystemhijri.cpp \
kcalendarsystemjalali.cpp \
tdeaboutdata.cpp \
kstringhandler.cpp \
tdecmdlineargs.cpp \
kurldrag.cpp \
kurl.cpp \
kidna.cpp \
kstaticdeleter.cpp \
kstandarddirs.cpp \
tdeconfig.cpp \
tdeconfigdialogmanager.cpp \
kcharsets.cpp \
tdeglobal.cpp \
kdebug.cpp \
tdetempfile.cpp \
ktempdir.cpp \
ksavefile.cpp \
tdeconfigbackend.cpp \
tdeconfigbase.cpp \
tdeconfigskeleton.cpp \
klockfile.cpp \
tdestdaccel.cpp \
kcheckaccelerators.cpp \
kkeyserver_x11.cpp \
kkeynative_x11.cpp \
tdeaccelbase.cpp \
tdeaccel.cpp \
tdeaccelmanager.cpp \
tdeshortcut.cpp \
tdeshortcutmenu.cpp \
tdeshortcutlist.cpp \
kinstance.cpp \
tdeversion.cpp \
tdelocale.cpp \
kicontheme.cpp \
kiconloader.cpp \
kiconeffect.cpp \
tdeglobalsettings.cpp \
kckey.cpp \
kglobalaccel.cpp \
kglobalaccel_win.cpp \
tdeaccelaction.cpp \
kuniqueapplication.cpp \
tdesycoca.cpp \
tdesycocadict.cpp \
tdesycocafactory.cpp \
tdeapplication.cpp \
kapplication_win.cpp \
kappdcopiface.cpp \
kprocctrl.cpp \
kdesktopfile.cpp \
kbufferedio.cpp \
netsupp_win32.cpp \
kasyncio.cpp \
ksockaddr.cpp \
kmdcodec.cpp \
kdcoppropertyproxy.cpp \
klibloader.cpp \
kprotocolinfo_tdecore.cpp \
../tdeio/tdeio/kprotocolinfo.cpp \
kprotocolinfofactory.cpp \
kmountpoint.cpp \
kmacroexpander.cpp \
kshell.cpp \
kclipboard.cpp \
kdebugdcopiface.cpp \
krandomsequence.cpp \
krfcdate.cpp \
tdemultipledrag.cpp \
kipc.cpp \
kuser.cpp \
kaudioplayer.cpp \
kvmallocator.cpp \
kqiodevicegzip_p.cpp

#network/kresolver.cpp \
#network/kresolvermanager.cpp \
#network/kreverseresolver.cpp \
#network/tdesocketaddress.cpp \
#network/tdesocketbase.cpp \
#network/tdesocketdevice.cpp \
#network/ksockssocketdevice.cpp

#tdestartupinfo.cpp \
#todo: kextsock.cpp \
#todo: ksock.cpp \
#todo: ksocks.cpp \
#kpath.cpp \

# generated:
SOURCES += \
kappdcopiface_stub.cpp \
kappdcopiface_skel.cpp \
tdesycoca_stub.cpp \
tdesycoca_skel.cpp \
kdebugdcopiface_stub.cpp \
kdebugdcopiface_skel.cpp


exists( custom_tdecore.pro ) {
  include( custom_tdecore.pro )
}

HEADERS		=
