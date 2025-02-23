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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/kjs
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore/network
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/kded
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

tde_create_translated_desktop(
  SOURCE proxyscout.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
  PO_DIR tdeio-desktops
)

tde_create_translated_desktop(
  SOURCE eventsrc
  DESTINATION ${DATA_INSTALL_DIR}/proxyscout
  PO_DIR tdeio-kpac-eventsrc
)


##### kded_proxyscout ###########################

set( target kded_proxyscout )

set( ${target}_SRCS
  proxyscout.cpp proxyscout.skel script.cpp
  downloader.cpp discovery.cpp
)

set( ${target}_SKELS
  proxyscout.h
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeinit_kded-shared kjs-shared ${RESOLV_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kpac_dhcp_helper ##########################

set( target kpac_dhcp_helper )

# FIXME on Gentoo this binary is not suided
tde_add_executable( ${target} SETUID
  SOURCES kpac_dhcp_helper.c
  DESTINATION ${BIN_INSTALL_DIR}
)
