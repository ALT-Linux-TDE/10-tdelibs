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
  ${CMAKE_SOURCE_DIR}/tdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### artsmessage ###############################

set( target artsmessage )

set( ${target}_SRCS
  artsmessage.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
