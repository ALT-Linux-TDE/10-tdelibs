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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install(FILES tdesasl.h DESTINATION ${INCLUDE_INSTALL_DIR}/tdeio )


##### tdesasl ###################################

set( target tdesasl )

set( ${target}_SRCS
  tdesasl.cpp
)

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 1.2.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
