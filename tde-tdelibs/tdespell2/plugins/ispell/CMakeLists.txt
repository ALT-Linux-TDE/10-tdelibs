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
  ${CMAKE_SOURCE_DIR}/tdespell2
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdespell_ispell.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdespell2-desktops
)


#### tdespell_ispell ##############################

set( target tdespell_ispell )

set( ${target}_SRCS
  tdespell_ispellclient.cpp tdespell_ispelldict.cpp
  correct.cpp good.cpp hash.cpp lookup.cpp makedent.cpp
  tgood.cpp ispell_checker.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdespell2-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
