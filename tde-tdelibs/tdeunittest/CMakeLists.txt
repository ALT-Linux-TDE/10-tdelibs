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

install( FILES runner.h tester.h module.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeunittest )


##### libtdeunittest ##############################

set( target tdeunittest )

set( ${target}_SRCS
  runner.cpp tester.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### tdeunittestmodrunner ########################

tde_add_executable( tdeunittestmodrunner
  SOURCES modrunner.cpp
  LINK tdeunittest-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
