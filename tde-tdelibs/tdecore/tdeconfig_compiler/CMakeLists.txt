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

add_subdirectory(tests)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### tdeconfig_compiler ##########################

tde_add_executable( tdeconfig_compiler
  SOURCES tdeconfig_compiler.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
