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
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### checks ####################################

tde_add_check_executable( dcop_test AUTOMOC
  SOURCES test.cpp ${CMAKE_CURRENT_BINARY_DIR}/test.skel
  LINK tdecore-shared
  DEPENDENCIES dcop_tests_generated
)

tde_add_check_executable( driver AUTOMOC
  SOURCES driver.cpp ${CMAKE_CURRENT_BINARY_DIR}/test.stub
  LINK tdecore-shared
  DEPENDENCIES dcop_tests_generated
)

add_test( NAME "dcop/dcop-tests"
    COMMAND sh -x "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" )

set_tests_properties( dcop/dcop-tests PROPERTIES TIMEOUT 30)


##### generated files ###########################

add_custom_target( dcop_tests_generated
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/test.h
    ${CMAKE_CURRENT_BINARY_DIR}/test_skel.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/test_stub.cpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.h ${CMAKE_CURRENT_BINARY_DIR}/shell.generated
  COMMAND perl "${CMAKE_CURRENT_SOURCE_DIR}/generate.pl"
    <"${CMAKE_CURRENT_SOURCE_DIR}/testcases"
  DEPENDS testcases generate.pl
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
