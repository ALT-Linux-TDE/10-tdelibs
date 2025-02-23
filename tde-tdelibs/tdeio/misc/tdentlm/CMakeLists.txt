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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install(FILES tdentlm.h DESTINATION ${INCLUDE_INSTALL_DIR}/tdeio )


##### tdentlm ###################################

set( target tdentlm )

set( ${target}_SRCS
  tdentlm.cpp des.cpp
)

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
