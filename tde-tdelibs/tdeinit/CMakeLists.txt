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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/libltdl
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${FREETYPE_INCLUDE_DIRS}
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


#### tdeinit ####################################

set( target tdeinit )

set( ${target}_SRCS
  tdeinit.cpp setproctitle.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kICE-static ltdlc-static tdeparts-shared ${FONTCONFIG_LIBRARIES} ${XFT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdeinit_wrapper ############################

set( target tdeinit_wrapper )

tde_add_executable( ${target}
  SOURCES wrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kshell #####################################

set( target kshell )

tde_add_executable( ${target}
  SOURCES shell.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdeinit_shutdown ###########################

set( target tdeinit_shutdown )

tde_add_executable( ${target}
  SOURCES wrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### lnusertemp #################################

set( target lnusertemp )

tde_add_executable( ${target}
  SOURCES lnusertemp.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kwrapper ###################################

set( target kwrapper )

tde_add_executable( ${target}
  SOURCES kwrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdeioslave ###################################

set( target tdeioslave )

tde_add_executable( ${target} AUTOMOC
  SOURCES tdeioslave.cpp
  LINK ltdlc-static tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdestartupconfig #############################

set( target tdestartupconfig )

tde_add_executable( ${target} AUTOMOC
  SOURCES tdestartupconfig.cpp
  LINK tdefakes-shared ${TQT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdedostartupconfig ###########################

set( target tdedostartupconfig )

tde_add_executable( ${target} AUTOMOC
  SOURCES tdedostartupconfig.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### start_tdeinit ##############################

set( target start_tdeinit )

if( TDEINIT_SETUID )
  set( _setuid SETUID )
endif( TDEINIT_SETUID )

tde_add_executable( ${target} ${_setuid}
  SOURCES start_tdeinit.c
  DESTINATION ${BIN_INSTALL_DIR}
)

set_property(
  TARGET ${target}
  APPEND PROPERTY COMPILE_DEFINITIONS
  EXECUTE="${BIN_INSTALL_DIR}/tdeinit"
)


#### start_tdeinit_wrapper ######################

set( target start_tdeinit_wrapper )

tde_add_executable( ${target}
  SOURCES start_tdeinit_wrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdelauncher ##################################

set( target tdelauncher )

set( ${target}_SRCS
  tdelauncher.cpp tdelauncher_main.cpp autostart.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
)
