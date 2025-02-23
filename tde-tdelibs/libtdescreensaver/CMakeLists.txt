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
  ${CMAKE_SOURCE_DIR}/tdecore
)

string( REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    tdescreensaver.h tdescreensaver_vroot.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### tdescreensaver ##############################

set( target tdescreensaver )

set( ${target}_SRCS
  main.cpp tdescreensaver.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
