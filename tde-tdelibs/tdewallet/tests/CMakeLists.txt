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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

add_definitions (
    -DKDE_NO_COMPAT
    -DTQT_NO_ASCII_CAST
)

tde_add_check_executable( tdewalletsync  AUTOMOC LINK tdewalletclient-shared )
tde_add_check_executable( tdewalletasync AUTOMOC
    SOURCES tdewalletasync.cpp tdewallettest.cpp
    LINK tdewalletclient-shared )
tde_add_check_executable( tdewalletaboth AUTOMOC
    SOURCES tdewalletboth.cpp tdewallettest.cpp
    LINK tdewalletclient-shared )
