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

add_definitions(
  -D_TDEPRINT_COMPILE
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeprint
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE lpdunix.print
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/plugins
  PO_DIR tdeprint-desktops
)


#### tdeprint_lpdunix ###########################

set( target tdeprint_lpdunix )

set( ${target}_SRCS
  kmlpdunixfactory.cpp kmlpdunixmanager.cpp
  klpdunixprinterimpl.cpp kmlpdunixuimanager.cpp
)

tde_add_kpart( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdeprint-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
