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
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    part.h plugin.h partmanager.h mainwindow.h dockmainwindow.h
    event.h browserextension.h factory.h historyprovider.h
    browserinterface.h genericfactory.h componentfactory.h
    browserrun.h statusbarextension.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeparts )


##### other data ################################

tde_create_translated_desktop(
  SOURCE
    kpart.desktop krop.desktop krwp.desktop browserview.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR tdeparts-desktops
)


#### tdeparts #####################################

set( target tdeparts )

set( ${target}_SRCS
  part.cpp plugin.cpp partmanager.cpp mainwindow.cpp
  dockmainwindow.cpp event.cpp browserextension.cpp
  factory.cpp  historyprovider.cpp browserinterface.cpp
  browserrun.cpp statusbarextension.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 2.1.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
