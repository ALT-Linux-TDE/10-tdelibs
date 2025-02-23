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
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kded
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE kdetrayproxy.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
)


##### kded_kdetrayproxy #########################

set( target kded_kdetrayproxy )

set( ${target}_SRCS
  kdetrayproxy.cpp module.cpp module.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdecore-shared tdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
