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
  -DKDE_NO_COMPAT
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdespell2
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    dialog.h highlighter.h configdialog.h configwidget.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdespell2 )


#### tdespell2 (static) ###########################

set( target tdespell2 )

set( ${target}_SRCS
   configwidget.cpp highlighter.cpp configdialog.cpp
   tdespell2ui.ui configui.ui
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)


#### tdespell2 ####################################

set( target tdespell2 )

set( ${target}_SRCS
  configwidget.cpp highlighter.cpp configdialog.cpp
  tdespell2ui.ui configui.ui
  dialog.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdespell2base-static tdeutils-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
