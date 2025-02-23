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
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/interfaces
)


link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdetexteditor_isearch.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR kate-desktops
)

install( FILES tdetexteditor_isearchui.rc DESTINATION ${DATA_INSTALL_DIR}/tdetexteditor_isearch )


##### tdetexteditor_isearch #######################

set( target tdetexteditor_isearch )

set( ${target}_SRCS
  ISearchPlugin.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdetexteditor-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
