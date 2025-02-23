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

add_subdirectory( dummy )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdeio/kssl
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdehtml
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/kssl
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### configure #################################

set( KJAVA_POLICYPATH "${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/kjava/-" )
configure_file( kjava.policy.in kjava.policy @ONLY )


##### install data ##############################

tde_create_translated_desktop(
  SOURCE kjavaappletviewer.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdehtml-desktops
)

install( FILES kjava.jar ${CMAKE_CURRENT_BINARY_DIR}/kjava.policy pluginsinfo DESTINATION ${DATA_INSTALL_DIR}/kjava )

tde_install_icons( DESTINATION ${DATA_INSTALL_DIR}/kjava/icons )


##### kjava-static ##############################

set( target kjava )

set( ${target}_SRCS
  kjavaapplet.cpp kjavaappletcontext.cpp kjavaappletserver.cpp
  kjavaappletwidget.cpp kjavaprocess.cpp kjavadownloader.cpp
)

tde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
  LINK ${TQT_LIBRARIES}
)


##### kjavaappletviewer #########################

set( target kjavaappletviewer )

set( ${target}_SRCS
  kjavaapplet.cpp kjavaappletcontext.cpp
  kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp
  kjavadownloader.cpp kjavaappletviewer.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
