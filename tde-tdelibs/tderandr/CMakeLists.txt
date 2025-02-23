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
  ${XRANDR_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${XRANDR_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    randr.h lowlevel_randr.h ktimerdialog.h libtderandr.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libtderandr )


##### tderandr ####################################

set( target tderandr )

set( ${target}_SRCS
  randr.cpp lowlevel_randr.c ktimerdialog.cpp libtderandr.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.95
  LINK tdeui-shared ${XRANDR_LIBRARIES} ${XEXT_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
