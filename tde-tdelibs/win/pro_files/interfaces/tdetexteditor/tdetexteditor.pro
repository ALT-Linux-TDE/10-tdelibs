TEMPLATE	= lib
DEFINES += MAKE_KTEXTEDITOR_LIB

include( $(KDELIBS)/win/common.pro )

LIBS += $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdefx$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeparts$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR\tdeio$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeabc$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR\kdewin32$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/interfaces $(KDELIBS)/interfaces/tdetexteditor \
 $(KDELIBS)/tdeabc

TARGET		= tdetexteditor$$KDEBUG

system( bash kmoc )
system( bash kdcopidl )

SOURCES = \
    tdetexteditor.cpp \
    editinterface.cpp editinterfaceext.cpp \
    clipboardinterface.cpp  selectioninterface.cpp searchinterface.cpp \
    codecompletioninterface.cpp wordwrapinterface.cpp blockselectioninterface.cpp \
    configinterface.cpp cursorinterface.cpp  dynwordwrapinterface.cpp \
    printinterface.cpp highlightinginterface.cpp markinterface.cpp \
    popupmenuinterface.cpp undointerface.cpp viewcursorinterface.cpp \
    editdcopinterface.cpp  clipboarddcopinterface.cpp \
    selectiondcopinterface.cpp  \
    searchdcopinterface.cpp  markinterfaceextension.cpp \
    configinterfaceextension.cpp encodinginterface.cpp sessionconfiginterface.cpp \
    viewstatusmsginterface.cpp  editorchooser.cpp \
    blockselectiondcopinterface.cpp documentinfo.cpp documentdcopinfo.cpp\
    encodingdcopinterface.cpp \
    printdcopinterface.cpp  \
    undodcopinterface.cpp viewcursordcopinterface.cpp \
    viewstatusmsgdcopinterface.cpp \
    selectioninterfaceext.cpp \
    texthintinterface.cpp variableinterface.cpp \
    templateinterface.cpp

# generated:
SOURCES += \
selectionextdcopinterface_skel.cpp \
viewstatusmsgdcopinterface_skel.cpp \
viewcursordcopinterface_skel.cpp \
undodcopinterface_skel.cpp \
printdcopinterface_skel.cpp \
encodingdcopinterface_skel.cpp \
documentdcopinfo_skel.cpp \
blockselectiondcopinterface_skel.cpp \
searchdcopinterface_skel.cpp \
selectiondcopinterface_skel.cpp \
clipboarddcopinterface_skel.cpp \
editdcopinterface_skel.cpp \
\
selectionextdcopinterface_stub.cpp \
viewstatusmsgdcopinterface_stub.cpp \
viewcursordcopinterface_stub.cpp \
undodcopinterface_stub.cpp \
printdcopinterface_stub.cpp \
encodingdcopinterface_stub.cpp \
documentdcopinfo_stub.cpp \
blockselectiondcopinterface_stub.cpp \
searchdcopinterface_stub.cpp \
selectiondcopinterface_stub.cpp \
clipboarddcopinterface_stub.cpp \
editdcopinterface_stub.cpp


HEADERS		= 

INTERFACES = \
editorchooser_ui.ui
