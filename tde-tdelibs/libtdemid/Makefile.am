
INCLUDES =  -I$(srcdir)/.. $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

libtdemidincludedir = $(includedir)/libtdemid
libtdemidinclude_HEADERS = midiout.h player.h track.h midimapper.h \
	midfile.h dattypes.h midistat.h deviceman.h synthout.h \
	fmout.h gusout.h alsaout.h voiceman.h notearray.h mt32togm.h \
	midispec.h libtdemid.h

lib_LTLIBRARIES = libtdemid.la
libtdemid_la_SOURCES  = midiout.cpp player.cpp track.cpp midimapper.cpp \
	midfile.cpp dattypes.cpp midistat.cpp deviceman.cpp synthout.cpp \
	fmout.cpp gusout.cpp alsaout.cpp voiceman.cpp mt32togm.cpp notearray.cpp \
	libtdemid.cpp

libtdemid_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 0:95  -no-undefined
libtdemid_la_LIBADD = $(LIBASOUND) ../tdecore/libtdecore.la $(LIB_TQT)

DOXYGEN_REFERENCES = tdecore
include ../admin/Doxyfile.am
