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
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

##### headers ###################################

install( FILES
    dispatcher.h dialog.h pluginpage.h componentsdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/ksettings )


##### ksettings #################################

set( target ksettings )

set( ${target}_SRCS
  dispatcher.cpp dialog.cpp pluginpage.cpp componentsdialog.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
