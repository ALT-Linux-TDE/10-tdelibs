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

add_subdirectory( KDE-ICE )
add_subdirectory( dcopidl )
add_subdirectory( dcopidlng )
add_subdirectory( dcopidl2cpp )
add_subdirectory( client )
add_subdirectory( tests )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### Define where your dcop binary is ##########

set( DCOP_PATH "${BIN_INSTALL_DIR}" )
configure_file( dcop-path.h.cmake dcop-path.h )


##### headers ###################################

install( FILES
    dcopclient.h dcopobject.h dcopstub.h dcopref.h
    dcoptypes.h kdatastream.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### libDCOP ###################################

set( target DCOP )

set( ${target}_SRCS
  dcopstub.cpp dcopref.cpp dcopobject.cpp dcopclient.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK kICE-static ${TQT_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)


#### dcopserver (tdeinit) #######################

set( target dcopserver )

set( ${target}_SRCS
  dcopserver.cpp dcopsignals.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kICE-static DCOP-shared
)


#### dcopserver_shutdown ########################

tde_add_executable( dcopserver_shutdown
  SOURCES dcopserver_shutdown.c
  DESTINATION ${BIN_INSTALL_DIR}
)
