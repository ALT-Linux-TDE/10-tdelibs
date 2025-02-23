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

if( ARTS_FOUND )
  add_subdirectory( tdefileaudiopreview )
endif( ARTS_FOUND )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/interfaces
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    player.h playerdcopobject.h view.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdemediaplayer )


##### other data ################################

tde_create_translated_desktop(
  SOURCE
    tdemediaplayerengine.desktop
    tdemediaplayer.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR interfaces-desktops
)


##### tdemediaplayer ##############################

set( target tdemediaplayer )

set( ${target}_SRCS
  player.cpp view.cpp playerdcopobject.skel
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK tdeparts-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
