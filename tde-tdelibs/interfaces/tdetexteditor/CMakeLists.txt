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

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/interfaces
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    document.h view.h editor.h plugin.h editinterface.h
    undointerface.h selectioninterface.h cursorinterface.h
    clipboardinterface.h popupmenuinterface.h viewcursorinterface.h
    searchinterface.h highlightinginterface.h blockselectioninterface.h
    codecompletioninterface.h configinterface.h markinterface.h
    printinterface.h wordwrapinterface.h dynwordwrapinterface.h
    markinterfaceextension.h configinterfaceextension.h
    encodinginterface.h viewstatusmsginterface.h
    sessionconfiginterface.h editorchooser.h documentinfo.h
    selectioninterfaceext.h selectionextdcopinterface.h
    texthintinterface.h editinterfaceext.h variableinterface.h
    templateinterface.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdetexteditor )


##### other data ################################

tde_create_translated_desktop(
  SOURCE
    tdetexteditor.desktop
    tdetexteditoreditor.desktop
    tdetexteditorplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR interfaces-desktops
)

tde_create_translated_desktop(
  SOURCE kcm_tdetexteditor.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kcm_componentchooser
  PO_DIR interfaces-desktops
)


##### tdetexteditor ###############################

set( target tdetexteditor )

set( ${target}_SRCS
  tdetexteditor.cpp
  editinterface.cpp clipboardinterface.cpp  selectioninterface.cpp searchinterface.cpp
  codecompletioninterface.cpp wordwrapinterface.cpp blockselectioninterface.cpp
  configinterface.cpp cursorinterface.cpp  dynwordwrapinterface.cpp
  printinterface.cpp highlightinginterface.cpp markinterface.cpp
  popupmenuinterface.cpp undointerface.cpp viewcursorinterface.cpp
  editdcopinterface.cpp editdcopinterface.skel clipboarddcopinterface.cpp
  clipboarddcopinterface.skel selectiondcopinterface.cpp selectiondcopinterface.skel
  searchdcopinterface.cpp searchdcopinterface.skel markinterfaceextension.cpp
  configinterfaceextension.cpp encodinginterface.cpp sessionconfiginterface.cpp
  viewstatusmsginterface.cpp editorchooser_ui.ui editorchooser.cpp
  blockselectiondcopinterface.cpp documentinfo.cpp documentdcopinfo.cpp
  blockselectiondcopinterface.skel documentdcopinfo.skel encodingdcopinterface.cpp
  encodingdcopinterface.skel printdcopinterface.cpp printdcopinterface.skel
  undodcopinterface.cpp undodcopinterface.skel viewcursordcopinterface.cpp
  viewcursordcopinterface.skel viewstatusmsgdcopinterface.cpp
  viewstatusmsgdcopinterface.skel selectioninterfaceext.cpp selectionextdcopinterface.skel
  texthintinterface.cpp editinterfaceext.cpp variableinterface.cpp templateinterface.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK tdeui-shared tdeparts-shared
  LINK_PRIVATE tdeabc-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
