#################################################
#
#  (C) 2010 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/tdeabc/vcard/include
  ${CMAKE_SOURCE_DIR}/tdeabc/vcard/include/generated
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/dcop
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

set( TDEABC_TESTS_LINK
  ${TQT_LIBRARIES}
  DCOP-shared
  tdecore-shared
  tdeui-shared
  tdefx-shared
  tdeio-shared
  tdetexteditor-shared
  tdeabc-shared
)

##### headers ###################################

install( FILES
    vcard.h vcardline.h vcardparser.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeabc )


##### vcards ####################################

set( target vcards )

set( ${target}_SRCS
  vcard.cpp vcardline.cpp vcardparser.cpp
)

tde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
)


##### test programs ##############################

tde_add_check_executable( testread AUTOMOC LINK ${TDEABC_TESTS_LINK} )
tde_add_check_executable( testwrite AUTOMOC LINK ${TDEABC_TESTS_LINK} )
tde_add_check_executable( testwrite2 AUTOMOC LINK ${TDEABC_TESTS_LINK} )
tde_add_check_executable( testvcardformat AUTOMOC LINK ${TDEABC_TESTS_LINK} )
tde_add_check_executable( testvcardformatimpl AUTOMOC LINK ${TDEABC_TESTS_LINK} )

tde_add_check_executable( testread2
  SOURCES testread2.cpp testutils.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

add_test( NAME tdeabc/vcardparser/testread2 COMMAND testread2 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" )

# testread tests
file( GLOB vcardparser_testfiles "tests/*.vcf" )
list( REMOVE_ITEM vcardparser_testfiles "${CMAKE_CURRENT_SOURCE_DIR}/tests/vcard5.vcf" )
list( SORT vcardparser_testfiles )

foreach(testfile ${vcardparser_testfiles})
  get_filename_component(testfile_name "${testfile}" NAME_WE )
  add_test( NAME tdeabc/vcardparser/testread-${testfile_name} COMMAND
    perl ${CMAKE_CURRENT_SOURCE_DIR}/checkvcard.pl ${testfile}
  )
endforeach(testfile)
