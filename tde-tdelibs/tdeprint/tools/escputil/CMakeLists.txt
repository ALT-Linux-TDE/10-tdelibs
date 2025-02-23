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
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE escputil.desktop
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/tools
  PO_DIR tdeprint-desktops
)


##### tdeprint_tool_escputil ####################

set( target tdeprint_tool_escputil )

set( ${target}_SRCS
  escpwidget.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
