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
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE kcookiejar.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
  PO_DIR tdeioslave-desktops
)

install( FILES kcookiescfg.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( FILES domain_info DESTINATION ${DATA_INSTALL_DIR}/tdehtml )


##### kcookiejar ################################

set( target kcookiejar )

set( ${target}_SRCS
  main.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdecore-shared
)


##### kded_kcookiejar ###########################

set( target kded_kcookiejar )

set( ${target}_SRCS
  kcookiejar.cpp kcookieserver.cpp kcookiewin.cpp
  kcookieserver.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeui-shared tdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
