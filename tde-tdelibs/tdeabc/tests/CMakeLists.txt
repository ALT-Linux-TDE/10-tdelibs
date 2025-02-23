#################################################
#
#  (C) 2016 Slávek Banko
#  slavek.banko (AT) axis.cz
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_BINARY_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/kab
  ${CMAKE_SOURCE_DIR}/tdeabc/plugins/sql
  ${CMAKE_SOURCE_DIR}/tdeabc/vcardparser
  ${CMAKE_SOURCE_DIR}/tdeabc/vcard/include
  ${CMAKE_SOURCE_DIR}/tdeabc/vcard/include/generated
  ${CMAKE_SOURCE_DIR}/tdeabc/vcardparser
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### test programs ##############################

tde_add_check_executable( testlock            AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testkabc            AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testkabcdlg         AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testdistlist        AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testaddressee       AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testaddresseelist   AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testaddressfmt      AUTOMOC LINK tdeabc-shared )
tde_add_check_executable( testaddresslineedit AUTOMOC LINK tdeabc-shared )

tde_add_check_executable( bigread  AUTOMOC LINK tdeabc_file-shared )
tde_add_check_executable( bigwrite AUTOMOC LINK tdeabc_file-shared )
# FIXME Skip build due to sql plugin is turned off permanently and unmaintained
# tde_add_check_executable( testdb   AUTOMOC LINK tdeabc_file-shared )
tde_add_check_executable( kabcargl AUTOMOC LINK tdeabc_file-shared )

tde_add_check_executable( testldapclient      AUTOMOC LINK tdeabc-shared )
