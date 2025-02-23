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
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


#### kded_kpasswdserver ##############################

set( target kded_kpasswdserver )

set( ${target}_SRCS
  kpasswdserver.cpp kpasswdserver.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
