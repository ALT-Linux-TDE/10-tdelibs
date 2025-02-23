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
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/tdeabc

  ${TQT_INCLUDE_DIRS}
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
    resourcenet.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeabc )


##### other data ################################

tde_create_translated_desktop(
  SOURCE net.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/tderesources/tdeabc
  PO_DIR tdeabc-desktops
)


##### tdeabc_net (library) ########################

set( target tdeabc_net )

set( ${target}_SRCS
  resourcenet.cpp resourcenetconfig.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdeabc-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### tdeabc_net (module) #########################

set( target tdeabc_net )

set( ${target}_SRCS
  resourcenetplugin.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeabc_net-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
