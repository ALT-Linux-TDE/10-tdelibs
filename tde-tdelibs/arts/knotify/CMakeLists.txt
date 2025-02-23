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
  ${ARTS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/arts/kde
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${ARTS_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE knotify.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)


##### knotify ###################################

set( target knotify )

set( ${target}_SRCS
  knotify.cpp knotify.skel
)

if( ARTS_FOUND )
  set( LIBS artskde-shared )
else( ARTS_FOUND )
  set( LIBS tdeui-shared )
endif( ARTS_FOUND )

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK ${LIBS}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
