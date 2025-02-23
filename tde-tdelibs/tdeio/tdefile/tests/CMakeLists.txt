#################################################
#
#  (C) 2016 Alexander Golubev
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_BINARY_DIR}/tdeio/tdefile
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)

tde_add_check_executable( kurlrequestertest     LINK tdeio-shared )
tde_add_check_executable( kopenwithtest         LINK tdeio-shared )
tde_add_check_executable( kdirselectdialogtest  LINK tdeio-shared )
tde_add_check_executable( kicondialogtest       LINK tdeio-shared )
tde_add_check_executable( kcustommenueditortest LINK tdeio-shared )
tde_add_check_executable( tdefilenotifytest     LINK tdeio-shared )

tde_add_check_executable( tdefiletreeviewtest   AUTOMOC LINK tdeio-shared )

tde_add_check_executable( kfstest AUTOMOC
    SOURCES kfstest.cpp kfdtest.cpp LINK tdeio-shared )
