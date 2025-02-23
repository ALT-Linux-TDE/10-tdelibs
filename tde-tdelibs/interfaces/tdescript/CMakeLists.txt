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

add_subdirectory( sample )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    scriptinterface.h scriptclientinterface.h scriptmanager.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

tde_create_translated_desktop(
  SOURCE scriptinterface.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR interfaces-desktops
)


##### tdescript ###################################

set( target tdescript )

set( ${target}_SRCS
  scriptmanager.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
