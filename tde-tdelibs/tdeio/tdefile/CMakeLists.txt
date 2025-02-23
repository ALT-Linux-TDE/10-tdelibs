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

add_subdirectory( tests )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/bookmarks
  ${LIBART_INCLUDE_DIRS}
)


##### headers ###################################

install( FILES
    tdefiledialog.h kencodingfiledialog.h
    tdediroperator.h tdefileview.h tdefilefiltercombo.h
    tdefiledetailview.h kcombiview.h kdiskfreesp.h
    tdefileiconview.h tderecentdocument.h
    kurlrequester.h tdefilepreview.h tdefile.h
    kurlcombobox.h kurlrequesterdlg.h kopenwith.h kpropsdlg.h
    kicondialog.h kdirsize.h kpreviewwidgetbase.h kimagefilepreview.h tdefilesharedlg.h
    tdefiletreeview.h tdefiletreeviewitem.h tdefiletreebranch.h
    kdirselectdialog.h kurlbar.h kpropertiesdialog.h knotifydialog.h
    kcustommenueditor.h ${CMAKE_CURRENT_BINARY_DIR}/knotifywidgetbase.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

tde_create_translated_desktop(
  SOURCE kpropsdlgplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR tdeio-desktops
)


##### tdefile #####################################

set( target tdefile )

set( ${target}_SRCS
  tdefilefiltercombo.cpp tdefileview.cpp tdefileiconview.cpp
  tderecentdocument.cpp tdefiledialog.cpp tdediroperator.cpp
  tdefiledetailview.cpp kcombiview.cpp kurlrequester.cpp
  tdefilepreview.cpp kurlcombobox.cpp kurlrequesterdlg.cpp
  kopenwith.cpp kpropertiesdialog.cpp kicondialog.cpp
  kdirsize.cpp tderecentdirs.cpp kdiskfreesp.cpp kimagefilepreview.cpp
  tdefilesharedlg.cpp kurlbar.cpp kmetaprops.cpp kpreviewprops.cpp
  tdefiletreeview.cpp tdefiletreeviewitem.cpp tdefiletreebranch.cpp
  kdirselectdialog.cpp tdefilebookmarkhandler.cpp tdefilemetainfowidget.cpp
  kcustommenueditor.cpp knotifywidgetbase.ui knotifydialog.cpp
  tdefilespeedbar.cpp kpreviewwidgetbase.cpp tdefilemetapreview.cpp
  kpropertiesdesktopbase.ui kpropertiesdesktopadvbase.ui
  kpropertiesmimetypebase.ui kencodingfiledialog.cpp
  kacleditwidget.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
