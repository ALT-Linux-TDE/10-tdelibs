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
  ${CMAKE_SOURCE_DIR}/tdespell2
  ${CMAKE_SOURCE_DIR}/tdecore
  ${ASPELL_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${ASPELL_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdespell_aspell.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdespell2-desktops
)


#### tdespell_aspell ##############################

set( target tdespell_aspell )

set( ${target}_SRCS
  tdespell_aspellclient.cpp tdespell_aspelldict.cpp
)

tde_add_kpart( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdespell2-shared ${ASPELL_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
