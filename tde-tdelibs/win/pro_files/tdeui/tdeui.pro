TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )


# needed to export library classes:
DEFINES += MAKE_TDEUI_LIB

LIBS += $$KDELIBDESTDIR\tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdefx$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX 

system( bash kmoc )
system( bash kdcopidl )

TARGET		= tdeui$$KDEBUG

SOURCES = \
tdeaboutapplication.cpp \
tdeaboutdialog.cpp \
tdeabouttde.cpp \
tdeactionclasses.cpp \
tdeactioncollection.cpp \
tdeaction.cpp \
tdeactionselector.cpp \
kactivelabel.cpp \
kanimwidget.cpp \
karrowbutton.cpp \
kauthicon.cpp \
kbugreport.cpp \
kbuttonbox.cpp \
kcharselect.cpp \
kcmenumngr.cpp \
tdecmodule.cpp \
kcolorbutton.cpp \
kcolorcombo.cpp \
kcolordialog.cpp \
kcolordrag.cpp \
kcombobox.cpp \
kcommand.cpp \
tdecompletionbox.cpp \
tdeconfigdialog.cpp \
kcursor.cpp \
kdatepicker.cpp \
kdatetbl.cpp \
kdatewidget.cpp \
kdialog.cpp \
kdialogbase.cpp \
kdockwidget.cpp \
kdockwidget_private.cpp \
kdualcolorbutton.cpp \
keditcl1.cpp \
keditcl2.cpp \
keditlistbox.cpp \
kedittoolbar.cpp \
tdefontcombo.cpp \
tdefontdialog.cpp \
tdefontrequester.cpp \
kguiitem.cpp \
khelpmenu.cpp \
kiconview.cpp \
kiconviewsearchline.cpp \
kjanuswidget.cpp \
kkeybutton.cpp \
kkeydialog.cpp \
klanguagebutton.cpp \
kled.cpp \
klineedit.cpp \
klineeditdlg.cpp \
tdelistbox.cpp \
tdelistview.cpp \
tdelistviewsearchline.cpp \
tdemainwindowiface.cpp \
tdemainwindow.cpp \
tdemenubar.cpp \
knuminput.cpp \
knumvalidator.cpp \
kpanelapplet.cpp \
kpanelappmenu.cpp \
kpanelextension.cpp \
kpanelmenu.cpp \
kpassdlg.cpp \
kpassivepopup.cpp \
kpixmapio.cpp \
kpixmapregionselectordialog.cpp \
kpixmapregionselectorwidget.cpp \
tdepopupmenu.cpp \
kprogress.cpp \
kpushbutton.cpp \
krestrictedline.cpp \
krootpixmap.cpp \
kruler.cpp \
ksconfig.cpp \
tdeselect.cpp \
kseparator.cpp \
tdeshortcutdialog.cpp \
tdespell.cpp \
tdespelldlg.cpp \
ksplashscreen.cpp \
ksqueezedtextlabel.cpp \
kstatusbar.cpp \
kstdaction.cpp \
kstdguiitem.cpp \
kstringvalidator.cpp \
ksyntaxhighlighter.cpp \
ksystemtray.cpp \
ktabctl.cpp \
ktextbrowser.cpp \
ktextedit.cpp \
ktip.cpp \
tdetoolbar.cpp \
tdetoolbarbutton.cpp \
tdetoolbarhandler.cpp \
tdetoolbarradiogroup.cpp \
kurllabel.cpp \
kwhatsthismanager.cpp \
twindowinfo.cpp \
kwizard.cpp \
kwordwrap.cpp \
kxmlguibuilder.cpp \
kxmlguiclient.cpp \
kxmlguifactory.cpp \
kxmlguifactory_p.cpp \
kdcopactionproxy.cpp \
ktabwidget.cpp \
ktabbar.cpp \
kdatetimewidget.cpp \
ktimewidget.cpp \
kinputdialog.cpp

exists( kmessagebox_win.cpp ) {
 #added KMessageBox::Dangerous implementation
 SOURCES += kmessagebox_win.cpp
}
!exists( kmessagebox_win.cpp ) {
 SOURCES += tdemessagebox.cpp
}

# generated:
SOURCES += \
tdemainwindowiface_stub.cpp \
tdemainwindowiface_skel.cpp

FORMS = \
tdeshortcutdialog_advanced.ui \
tdeshortcutdialog_simple.ui \
tdespellui.ui

HEADERS =
