#################################################
#
#  (C) 2010 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

if( BUILD_TDEUI_TESTS )
  add_subdirectory( tests )
endif( BUILD_TDEUI_TESTS )

add_subdirectory( about )
add_subdirectory( kdetrayproxy )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${FREETYPE_INCLUDE_DIRS}
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kprogressbox.h kprogress.h kcolordlg.h
    kcolordialog.h tdeselect.h
    kdatepik.h kdatepicker.h kdatetbl.h tdefontdialog.h tdepopupmenu.h
    tdefontrequester.h ktabctl.h kstatusbar.h
    tdemainwindow.h tdemainwindowiface.h tdetoolbar.h tdemenubar.h
    knuminput.h kseparator.h klineedit.h
    krestrictedline.h kcolorbutton.h kcolorbtn.h
    ksystemtray.h kdockwindow.h kbuttonbox.h
    keditcl.h kled.h keditlistbox.h
    kwizard.h kkeydialog.h kkeybutton.h
    kurllabel.h kruler.h kcursor.h
    klineeditdlg.h kinputdialog.h kactivelabel.h
    kcharselect.h kcolordrag.h qxembed.h
    knumvalidator.h kdialog.h kdialogbase.h
    kjanuswidget.h tdeaboutdialog.h
    kauthicon.h tdemessagebox.h ksharedpixmap.h
    kdualcolorbtn.h kdualcolorbutton.h tdetoolbarbutton.h
    tdetoolbarradiogroup.h ktextbrowser.h
    tdeaction.h tdeactioncollection.h tdeactionclasses.h khelpmenu.h kswitchlanguagedialog.h
    tdecmodule.h kcmenumngr.h kpanelmenu.h kpanelappmenu.h
    tdeactionshortcutlist.h kstdaction.h kcombobox.h
    kiconview.h tdelistview.h tdelistbox.h kbugreport.h kpassdlg.h
    kxmlguiclient.h kxmlgui.h kxmlguibuilder.h kxmlguifactory.h
    kpixmapio.h kwordwrap.h
    kedittoolbar.h kdockwidget.h kanimwidget.h
    krootpixmap.h tdeabouttde.h tdeaboutapplication.h
    kpanelapplet.h kpushbutton.h kdcopactionproxy.h kcolorcombo.h
    kpanelextension.h tdecompletionbox.h ksqueezedtextlabel.h
    kcommand.h twindowlistmenu.h tdefontcombo.h tdeaccelgen.h ktip.h
    kdatewidget.h karrowbutton.h kguiitem.h kstdguiitem.h
    kstringvalidator.h twindowinfo.h
    ktextedit.h kpassivepopup.h tdepassivepopupstack.h
    kdatetimewidget.h ktimewidget.h ktimezonewidget.h kscrollview.h
    tdespell.h ksconfig.h tdespelldlg.h ksyntaxhighlighter.h
    ktabbar.h ktabwidget.h ksplashscreen.h tdeconfigdialog.h
    tdeactionselector.h klanguagebutton.h tdelistviewsearchline.h
    kpixmapregionselectorwidget.h kpixmapregionselectordialog.h
    kiconviewsearchline.h tdetoolbarlabelaction.h tdeshortcutdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

install( FILES
    abouttde.png ktip-bulb.png ktip-background.png
  DESTINATION ${DATA_INSTALL_DIR}/tdeui/pics )

install( FILES
    ui_standards.rc
  DESTINATION ${CONFIG_INSTALL_DIR}/ui )

install( PROGRAMS
    preparetips
  DESTINATION ${BIN_INSTALL_DIR} )


##### libtdeui ##################################

set( target tdeui )

set( ${target}_SRCS
  qxembed.cpp ksharedpixmap.cpp kpixmapio.cpp
  tdepopupmenu.cpp tdetoolbar.cpp tdeaction.cpp kstdaction.cpp
  tdeactioncollection.cpp tdeactionclasses.cpp
  kurllabel.cpp tdemenubar.cpp kinputdialog.cpp
  knuminput.cpp klineedit.cpp tdelistview.cpp kprogress.cpp
  kprogressbox.cpp kcolordialog.cpp tdeselect.cpp kdatepicker.cpp
  kdatetbl.cpp tdefontrequester.cpp tdefontdialog.cpp ktabctl.cpp
  kstatusbar.cpp tdemainwindow.cpp
  keditlistbox.cpp kscrollview.cpp
  kseparator.cpp krestrictedline.cpp
  kcolorbutton.cpp ksystemtray.cpp
  kbuttonbox.cpp keditcl1.cpp keditcl2.cpp kled.cpp
  tdeshortcutdialog_simple.ui tdeshortcutdialog_advanced.ui
  tdeshortcutdialog.cpp kkeybutton.cpp kkeydialog.cpp
  kruler.cpp kcursor.cpp klineeditdlg.cpp
  kcharselect.cpp kcolordrag.cpp
  knumvalidator.cpp kdialog.cpp kdialogbase.cpp
  kjanuswidget.cpp tdeaboutdialog.cpp
  kauthicon.cpp tdemessagebox.cpp kdualcolorbutton.cpp
  tdetoolbarradiogroup.cpp tdetoolbarbutton.cpp
  ktextbrowser.cpp khelpmenu.cpp kswitchlanguagedialog.cpp
  kcmenumngr.cpp kpanelmenu.cpp
  kiconview.cpp tdelistbox.cpp kwordwrap.cpp kbugreport.cpp
  kcombobox.cpp kpassdlg.cpp kxmlguiclient.cpp kxmlguifactory.cpp
  kxmlguifactory_p.cpp kxmlguibuilder.cpp kedittoolbar.cpp
  kpanelappmenu.cpp kdockwidget.cpp kdockwidget_private.cpp
  kanimwidget.cpp krootpixmap.cpp tdeabouttde.cpp
  tdeaboutapplication.cpp kpanelapplet.cpp kdcopactionproxy.cpp
  kcolorcombo.cpp kpushbutton.cpp kpanelextension.cpp
  tdecompletionbox.cpp ksqueezedtextlabel.cpp tdecmodule.cpp
  kcommand.cpp twindowlistmenu.cpp tdefontcombo.cpp ktip.cpp
  kdatewidget.cpp karrowbutton.cpp tdemainwindowiface.cpp
  tdemainwindowiface.skel kguiitem.cpp kstdguiitem.cpp
  kstringvalidator.cpp kactivelabel.cpp
  twindowinfo.cpp ktextedit.cpp kwizard.cpp kpassivepopup.cpp
  tdepassivepopupstack.cpp tdetoolbarhandler.cpp kdatetimewidget.cpp
  ktimewidget.cpp ktimezonewidget.cpp ksconfig.cpp tdespelldlg.cpp
  tdespell.cpp ksyntaxhighlighter.cpp ktabbar.cpp ktabwidget.cpp
  ksplashscreen.cpp tdespellui.ui tdeconfigdialog.cpp
  tdeactionselector.cpp klanguagebutton.cpp tdelistviewsearchline.cpp
  kpixmapregionselectorwidget.cpp kpixmapregionselectordialog.cpp
  kwhatsthismanager.cpp kiconviewsearchline.cpp
  tdetoolbarlabelaction.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK tdecore-shared
  LINK_PRIVATE ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${XEXT_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)


##### libtdespell #################################

set( target tdespell )

configure_file( ${TDE_CMAKE_TEMPLATES}/tde_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_library( ${target} SHARED
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
  LINK ${TQT_LIBRARIES}
  VERSION 14.0.0
  DESTINATION ${LIB_INSTALL_DIR}
)
