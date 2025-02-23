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
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/tdewallet/backend
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdewalletd.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
  PO_DIR tdeio-desktops
)


#### kded_tdewalletd ##############################

set( target kded_tdewalletd )

set( ${target}_SRCS
  tdewalletd.cpp tdewalletd.skel ktimeout.cpp tdewalletwizard.ui
  kbetterthankdialogbase.ui
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeinit_kded-shared tdewalletbackend-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
