
#	This file is part of the KDE libraries
#    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
#			  (C) 1997 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

SUBDIRS = . tests about kdetrayproxy

INCLUDES= -I/usr/include/freetype2/ -I$(top_srcdir)/tdefx -I$(top_srcdir)/interfaces $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

lib_LTLIBRARIES = libtdeui.la libtdespell.la
libtdeui_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined -version-info 6:0:2
libtdeui_la_LIBADD = ../tdecore/libtdecore.la $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_X11) $(top_builddir)/tdefx/libtdefx.la $(LIB_XEXT)

libtdespell_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 6:0:2 -no-undefined
libtdespell_la_SOURCES = dummy.cpp

picsdir = $(kde_datadir)/tdeui/pics
pics_DATA = abouttde.png ktip-bulb.png ktip-background.png

include_HEADERS = kprogressbox.h kprogress.h kcolordlg.h \
		kcolordialog.h tdeselect.h \
		kdatepik.h kdatepicker.h kdatetbl.h tdefontdialog.h tdepopupmenu.h \
		tdefontrequester.h ktabctl.h kstatusbar.h \
		tdemainwindow.h tdemainwindowiface.h tdetoolbar.h tdemenubar.h \
		knuminput.h kseparator.h klineedit.h \
		krestrictedline.h kcolorbutton.h kcolorbtn.h \
		ksystemtray.h kdockwindow.h kbuttonbox.h \
		keditcl.h kled.h keditlistbox.h \
		kwizard.h kkeydialog.h kkeybutton.h \
		kurllabel.h kruler.h kcursor.h \
		klineeditdlg.h kinputdialog.h kactivelabel.h \
		kcharselect.h kcolordrag.h qxembed.h \
		knumvalidator.h kdialog.h kdialogbase.h \
		kjanuswidget.h tdeaboutdialog.h \
		kauthicon.h tdemessagebox.h ksharedpixmap.h \
		kdualcolorbtn.h kdualcolorbutton.h tdetoolbarbutton.h \
		tdetoolbarradiogroup.h ktextbrowser.h \
		tdeaction.h tdeactioncollection.h tdeactionclasses.h khelpmenu.h kswitchlanguagedialog.h \
		tdecmodule.h kcmenumngr.h kpanelmenu.h kpanelappmenu.h \
		tdeactionshortcutlist.h kstdaction.h kcombobox.h \
		kiconview.h tdelistview.h tdelistbox.h kbugreport.h kpassdlg.h \
		kxmlguiclient.h kxmlgui.h kxmlguibuilder.h kxmlguifactory.h \
		kpixmapio.h kwordwrap.h \
		kedittoolbar.h kdockwidget.h kanimwidget.h \
		krootpixmap.h tdeabouttde.h tdeaboutapplication.h \
		kpanelapplet.h kpushbutton.h kdcopactionproxy.h kcolorcombo.h \
		kpanelextension.h tdecompletionbox.h ksqueezedtextlabel.h \
		kcommand.h twindowlistmenu.h tdefontcombo.h tdeaccelgen.h ktip.h \
		kdatewidget.h karrowbutton.h kguiitem.h kstdguiitem.h \
		kstringvalidator.h twindowinfo.h \
		ktextedit.h kpassivepopup.h \
		kdatetimewidget.h ktimewidget.h ktimezonewidget.h kscrollview.h \
		tdespell.h ksconfig.h tdespelldlg.h ksyntaxhighlighter.h \
		ktabbar.h ktabwidget.h ksplashscreen.h tdeconfigdialog.h \
		tdeactionselector.h klanguagebutton.h tdelistviewsearchline.h \
		kpixmapregionselectorwidget.h kpixmapregionselectordialog.h \
		kiconviewsearchline.h tdetoolbarlabelaction.h tdeshortcutdialog.h

# the order of the sources isn't randomly (at least not completely).
# the order is predictated by the use of X11 header files
# the files on the first line need to occur first when compiling
# with --enable-final since they use X defines that get undefined
# in ksharedpixmap.cpp
libtdeui_la_SOURCES = \
		qxembed.cpp ksharedpixmap.cpp kpixmapio.cpp \
		tdepopupmenu.cpp tdetoolbar.cpp tdeaction.cpp kstdaction.cpp \
		tdeactioncollection.cpp tdeactionclasses.cpp \
		kurllabel.cpp tdemenubar.cpp kinputdialog.cpp \
		knuminput.cpp klineedit.cpp tdelistview.cpp kprogress.cpp \
		kprogressbox.cpp kcolordialog.cpp tdeselect.cpp kdatepicker.cpp \
		kdatetbl.cpp tdefontrequester.cpp tdefontdialog.cpp ktabctl.cpp \
		kstatusbar.cpp tdemainwindow.cpp \
		keditlistbox.cpp kscrollview.cpp \
		kseparator.cpp krestrictedline.cpp  \
		kcolorbutton.cpp ksystemtray.cpp \
		kbuttonbox.cpp keditcl1.cpp keditcl2.cpp kled.cpp \
		tdeshortcutdialog_simple.ui tdeshortcutdialog_advanced.ui \
		tdeshortcutdialog.cpp kkeybutton.cpp kkeydialog.cpp \
		kruler.cpp kcursor.cpp klineeditdlg.cpp  \
		kcharselect.cpp kcolordrag.cpp  \
		knumvalidator.cpp kdialog.cpp kdialogbase.cpp \
		kjanuswidget.cpp tdeaboutdialog.cpp \
		kauthicon.cpp tdemessagebox.cpp kdualcolorbutton.cpp \
		tdetoolbarradiogroup.cpp tdetoolbarbutton.cpp \
		ktextbrowser.cpp khelpmenu.cpp kswitchlanguagedialog.cpp \
		kcmenumngr.cpp kpanelmenu.cpp \
		kiconview.cpp tdelistbox.cpp kwordwrap.cpp kbugreport.cpp \
		kcombobox.cpp kpassdlg.cpp kxmlguiclient.cpp kxmlguifactory.cpp \
		kxmlguifactory_p.cpp kxmlguibuilder.cpp kedittoolbar.cpp \
		kpanelappmenu.cpp kdockwidget.cpp kdockwidget_private.cpp \
		kanimwidget.cpp krootpixmap.cpp tdeabouttde.cpp \
		tdeaboutapplication.cpp kpanelapplet.cpp kdcopactionproxy.cpp \
		kcolorcombo.cpp kpushbutton.cpp kpanelextension.cpp \
		tdecompletionbox.cpp ksqueezedtextlabel.cpp tdecmodule.cpp \
		kcommand.cpp twindowlistmenu.cpp tdefontcombo.cpp ktip.cpp \
		kdatewidget.cpp karrowbutton.cpp tdemainwindowiface.cpp \
                tdemainwindowiface.skel kguiitem.cpp kstdguiitem.cpp \
		kstringvalidator.cpp kactivelabel.cpp \
		twindowinfo.cpp ktextedit.cpp kwizard.cpp kpassivepopup.cpp \
		tdetoolbarhandler.cpp kdatetimewidget.cpp ktimewidget.cpp ktimezonewidget.cpp \
		ksconfig.cpp tdespelldlg.cpp \
		tdespell.cpp ksyntaxhighlighter.cpp ktabbar.cpp ktabwidget.cpp \
		ksplashscreen.cpp tdespellui.ui tdeconfigdialog.cpp \
		tdeactionselector.cpp klanguagebutton.cpp tdelistviewsearchline.cpp \
		kpixmapregionselectorwidget.cpp kpixmapregionselectordialog.cpp \
		kwhatsthismanager.cpp kiconviewsearchline.cpp \
		tdetoolbarlabelaction.cpp

noinst_HEADERS = kdockwidget_private.h kdockwidget_p.h tdelistviewlineedit.h \
		kdialogbase_priv.h kaboutdialog_private.h kcursor_private.h \
		kdepackages.h kxmlguifactory_p.h \
		kstdaction_p.h kwhatsthismanager_p.h

scripts_SCRIPTS = preparetips
scriptsdir = $(kde_bindir)

libtdeui_la_METASOURCES = AUTO
libtdeui_la_NMCHECK = $(srcdir)/libtdeui.nmcheck
libtdeui_la_NMCHECKWEAK = $(srcdir)/libtdeui_weak.nmcheck $(top_srcdir)/tdecore/libtdecore_weak.nmcheck \
	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/tdecore/libtqt-mt_weak.nmcheck

rcdir = $(kde_confdir)/ui
rc_DATA = ui_standards.rc

EXTRA_DIST = CHANGES.kdatepicker USERS.kdatepicker kpixmapio-perf.png dcolorreset.xpm kpartgui.dtd dcolorarrow.xbm colors Mainpage.dox

.PHONY: kdepackages

kdepackages:
	-rm $(srcdir)/kdepackages.h
	curl "http://bugsold.kde.org/xml.cgi?data=versiontable" > tmp_download
	echo "// DO NOT EDIT - EDIT bugs/Maintainers instead" >  $(srcdir)/kdepackages.h
	echo "const char * const packages[] = {" >> $(srcdir)/kdepackages.h
	cat tmp_download | egrep "product|component" | grep -v "</product>" | \
	sed -e "s,<product name=\"\([^\"]*\)\">,product=\"\1\"; component=\"general\",; \
	s#<component>\([^<]*\)</component>#component=\"\1\"#"  > tmp_download2
	(while read i; do eval $$i; echo "\"$$product/$$component\","; done) < tmp_download2 | \
	sed -e "s,/general\",/00general\"," | LC_ALL=C sort -u | \
	grep -v 'valgrind/' | sed -e "s,/00general\",\"," >> $(srcdir)/kdepackages.h
	echo "0 };" >>  $(srcdir)/kdepackages.h
	rm tmp_download2 tmp_download

dummy.cpp:
	echo "#ifdef _AIX" >dummy.cpp
	echo "namespace {" >> dummy.cpp
	echo "void *not_empty_file;" >>dummy.cpp
	echo "}" >> dummy.cpp
	echo "#endif" >> dummy.cpp

DOXYGEN_REFERENCES = tdecore tdefx dcop tdeio tdeutils
DOXYGEN_SET_INTERNAL_DOCS = YES
include ../admin/Doxyfile.am
