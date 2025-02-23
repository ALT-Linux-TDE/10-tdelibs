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
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### dcopidl2cpp ###############################

set( target dcopidl2cpp )

set( ${target}_SRCS
  main.cpp skel.cpp stub.cpp stubimpl.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK ${TQT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
