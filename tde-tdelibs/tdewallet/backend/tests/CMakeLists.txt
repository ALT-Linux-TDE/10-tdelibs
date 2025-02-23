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
  ${CMAKE_SOURCE_DIR}/tdewallet/backend
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

tde_add_check_executable( backendtest AUTOMOC
    LINK tdewalletbackend-shared tdewalletclient-shared )
add_test( NAME "tdewallet/backed/backendtest"
    COMMAND sh -x "${CMAKE_CURRENT_SOURCE_DIR}/backendtest.sh" )
set_tests_properties( "tdewallet/backed/backendtest" PROPERTIES TIMEOUT 30)
tde_add_check_executable( testbf AUTOMOC
    SOURCES testbf.cpp ../blockcipher.cpp ../blowfish.cpp
    LINK tdewalletbackend-shared tdewalletclient-shared TEST )
tde_add_check_executable( testsha AUTOMOC
    SOURCES testsha.cpp ../sha1.cpp
    LINK tdewalletbackend-shared tdewalletclient-shared TEST )
