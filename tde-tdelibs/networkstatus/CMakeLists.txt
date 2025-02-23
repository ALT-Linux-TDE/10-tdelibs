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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/kded
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    serviceiface.h provideriface.h connectionmanager.h
    networkstatuscommon.h networkstatusindicator.h
    networkstatusiface.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

tde_create_translated_desktop(
  SOURCE networkstatus.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
)


##### networkstatus #############################

set( target networkstatus )

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES networkstatuscommon.cpp
  VERSION 0.0.0
  LINK  tdeui-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### connectionmanager #########################

set( target connectionmanager )

set( ${target}_SRCS
  connectionmanager.cpp networkstatusindicator.cpp
  connectionmanager.skel clientiface.stub networkstatusiface.stub
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK networkstatus-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kded_networkstatus ########################

set( target kded_networkstatus )

set( ${target}_SRCS
  networkstatus.cpp network.cpp networkstatus.skel
  clientiface.skel serviceiface.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeinit_kded-shared networkstatus-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### networkstatustestservice ##################

set( target networkstatustestservice )

set( ${target}_SRCS
  testservice.cpp provideriface.skel serviceiface.stub
)

tde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK networkstatus-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
