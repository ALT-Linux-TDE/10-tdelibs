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

add_subdirectory( icons )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    tdeimproxy.h tdeimproxyiface.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### tdeimproxy ##################################

set( target tdeimproxy )

set( ${target}_SRCS
  tdeimproxy.cpp tdeimproxyiface.skel
  ${CMAKE_SOURCE_DIR}/interfaces/tdeimproxy/interface/kimiface.stub
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
