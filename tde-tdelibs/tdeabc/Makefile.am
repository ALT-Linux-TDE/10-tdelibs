SUBDIRS = vcard vcardparser . formats plugins scripts tests

# Make sure $(all_includes) remains last!
INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc -I$(top_srcdir)/kab \
           -I$(srcdir)/vcard/include -I$(srcdir)/vcardparser/ \
           -I$(srcdir)/vcard/include/generated \
           -I$(srcdir)/vcardparser $(all_includes)

field.cpp: addressee.h addressee.cpp
addressee.cpp: addressee.h
addressee.cpp addressee.h field.cpp: \
				$(srcdir)/scripts/makeaddressee \
				$(srcdir)/scripts/addressee.src.cpp \
				$(srcdir)/scripts/addressee.src.h \
				$(srcdir)/scripts/entrylist \
				$(srcdir)/scripts/field.src.cpp
	mysrcdir=`cd $(srcdir)/scripts && pwd` ;\
	cd scripts && $(PERL) $$mysrcdir/makeaddressee

CLEANFILES = addressee.h addressee.cpp field.cpp

lib_LTLIBRARIES = libtdeabc.la
libtdeabc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 3:0:2 -no-undefined
libtdeabc_la_LIBADD = vcard/libvcard.la vcardparser/libvcards.la $(LIB_TDEIO) \
                    $(top_builddir)/tderesources/libtderesources.la $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDEUI) $(LIB_TDECORE)
libtdeabc_la_COMPILE_FIRST = addressee.h

libtdeabc_la_SOURCES = \
  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp agent.cpp \
  distributionlist.cpp distributionlistdialog.cpp distributionlisteditor.cpp \
  errorhandler.cpp field.cpp formatfactory.cpp geo.cpp key.cpp \
  phonenumber.cpp picture.cpp plugin.cpp resource.cpp \
  resourceselectdialog.cpp secrecy.cpp sound.cpp stdaddressbook.cpp \
  timezone.cpp vcard21parser.cpp vcardconverter.cpp vcardformat.cpp \
  vcardformatimpl.cpp vcardformatplugin.cpp ldifconverter.cpp addresslineedit.cpp \
  ldapclient.cpp addresseelist.cpp vcardtool.cpp addresseehelper.cpp \
  addresseehelper.skel lock.cpp locknull.cpp ldif.cpp ldapurl.cpp ldapconfigwidget.cpp \
  sortmode.cpp


tdeabcincludedir = $(includedir)/tdeabc
tdeabcinclude_HEADERS = address.h addressbook.h addressee.h addresseedialog.h \
  agent.h distributionlist.h distributionlistdialog.h distributionlisteditor.h \
  errorhandler.h field.h format.h formatfactory.h formatplugin.h geo.h key.h \
  phonenumber.h picture.h plugin.h resource.h \
  resourceselectdialog.h secrecy.h sound.h stdaddressbook.h timezone.h \
  vcardconverter.h vcardformat.h vcardformatplugin.h ldifconverter.h \
  addresslineedit.h ldapclient.h addresseelist.h lock.h locknull.h ldif.h \
  ldapurl.h ldapconfigwidget.h sortmode.h

METASOURCES = AUTO

bin_PROGRAMS = tdeab2tdeabc

tdeab2tdeabc_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdeab2tdeabc_LDADD = libtdeabc.la ../kab/libkab.la
tdeab2tdeabc_SOURCES = tdeab2tdeabc.cpp

autostart_DATA = tdeab2tdeabc.desktop
autostartdir = $(datadir)/autostart

manager_DATA = tdeabc_manager.desktop
managerdir = $(kde_servicesdir)/tderesources

DOXYGEN_REFERENCES = tdecore tdeui

map_DATA = countrytransl.map
mapdir = $(kde_datadir)/tdeabc

EXTRA_DIST = $(map_DATA)

include ../admin/Doxyfile.am
