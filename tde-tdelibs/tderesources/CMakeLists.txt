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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    resource.h configwidget.h factory.h manageriface.h
    managerimpl.h manager.h selectdialog.h configdialog.h
    configpage.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tderesources )


##### other data ################################

tde_create_translated_desktop(
  SOURCE tderesources_plugin.desktop tderesources_manager.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR tderesources-desktops
)

tde_create_translated_desktop(
  SOURCE tderesources.desktop
  PO_DIR tderesources-desktops
)


##### tderesources ################################

set( target tderesources )

set( ${target}_SRCS
  resource.cpp factory.cpp managerimpl.cpp configwidget.cpp
  configdialog.cpp selectdialog.cpp configpage.cpp
  manageriface.skel manageriface.stub
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.2.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kcm_tderesources ############################

set( target kcm_tderesources )

set( ${target}_SRCS
  kcmtderesources.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tderesources-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
