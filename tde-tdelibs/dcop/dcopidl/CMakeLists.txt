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
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### dcopidl ###################################

set( target dcopidl )

set( ${target}_SRCS
  main.cpp yacc.cpp scanner.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK DCOP-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
