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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${LIBART_INCLUDE_DIRS}
)


##### headers ###################################

install( FILES
    ksvgiconengine.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### kdesvgicons ###############################

set( target kdesvgicons )

set( ${target}_SRCS
  ksvgiconengine.cpp ksvgiconpainter.cpp
)

tde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
  LINK ${LIBART_LIBRARIES}
)
