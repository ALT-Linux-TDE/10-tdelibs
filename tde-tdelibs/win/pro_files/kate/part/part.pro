TEMPLATE	= lib
CONFIG += trinitylib #this is a kde module library

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_KATEPART_LIB

TARGET		= katepart$$KDELIBDEBUG

LIBS += $$KDELIBDESTDIR/katepartinterfaces$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdetexteditor$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdecore$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/tdeutils$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeparts$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/tdeio$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/interfaces $(KDELIBS)/interfaces/kregexpeditor \
	$(KDELIBS)/tdeutils $(KDELIBS)/tdeprint

system( bash kmoc )
system( bash kdcopidl )

SOURCES = \
  katesearch.cpp katebuffer.cpp katecmds.cpp \
  kateundo.cpp katecursor.cpp katedialogs.cpp katedocument.cpp \
  katefactory.cpp katehighlight.cpp katesyntaxdocument.cpp \
  katetextline.cpp kateview.cpp kateconfig.cpp kateviewhelpers.cpp \
  katecodecompletion.cpp katedocumenthelpers.cpp \
  katecodefoldinghelpers.cpp kateviewinternal.cpp katebookmarks.cpp \
  katefont.cpp katelinerange.cpp katesupercursor.cpp \
  katearbitraryhighlight.cpp katerenderer.cpp kateattribute.cpp \
  kateindentscriptabstracts.cpp \
  kateautoindent.cpp katefiletype.cpp kateschema.cpp \
  katetemplatehandler.cpp \
  kateprinter.cpp katespell.cpp

#todo:	katejscript.cpp

HEADERS =

# generated:
SOURCES += \
katedocument_skel.cpp
