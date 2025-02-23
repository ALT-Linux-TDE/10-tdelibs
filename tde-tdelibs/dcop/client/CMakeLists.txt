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
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


#### dcop #######################################

tde_add_executable( dcop SOURCES dcop.cpp LINK DCOP-shared X11 DESTINATION ${BIN_INSTALL_DIR} )


#### dcopfind ###################################

tde_add_executable( dcopfind SOURCES dcopfind.cpp LINK DCOP-shared DESTINATION ${BIN_INSTALL_DIR} )


#### dcopclient #################################

tde_add_executable( dcopclient SOURCES dcopclient.c DESTINATION ${BIN_INSTALL_DIR} )


#### dcopobject #################################

tde_add_executable( dcopobject SOURCES dcopobject.c DESTINATION ${BIN_INSTALL_DIR} )


#### dcopref ####################################

tde_add_executable( dcopref SOURCES dcopref.c DESTINATION ${BIN_INSTALL_DIR} )


#### dcopstart ##################################

tde_add_executable( dcopstart SOURCES dcopstart.cpp LINK DCOP-shared DESTINATION ${BIN_INSTALL_DIR} )


#### dcopquit ###################################

tde_add_executable( dcopquit SOURCES dcopquit.cpp LINK DCOP-shared X11 DESTINATION ${BIN_INSTALL_DIR} )
