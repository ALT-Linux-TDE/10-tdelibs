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

add_subdirectory( tdesendbugmail )
add_subdirectory( kpac )
add_subdirectory( tdesasl )
add_subdirectory( kssld )
add_subdirectory( tdefile )
add_subdirectory( tdewalletd )
add_subdirectory( tdentlm )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdeio
  ${CMAKE_BINARY_DIR}/tdeio/tdeio
  ${CMAKE_BINARY_DIR}/tdeio/kssl
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/kssl
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES uiserver.h DESTINATION ${INCLUDE_INSTALL_DIR}/tdeio )


##### other data ################################

tde_create_translated_desktop(
  SOURCE
    tdeio_uiserver.desktop tdemailservice.protocol
    telnet.protocol rlogin.protocol rtsp.protocol
    ssh.protocol mms.protocol mmst.protocol mmsu.protocol
    pnm.protocol rtspt.protocol rtspu.protocol
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdeio-desktops
)

install( FILES fileshareset DESTINATION ${BIN_INSTALL_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE SETUID )


#### tdeio_uiserver ###############################

set( target tdeio_uiserver )

set( ${target}_SRCS
  uiserver.cpp uiserver.skel
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
)


##### tdemailservice ##############################

set( target tdemailservice )

tde_add_executable( ${target}
  SOURCES ${target}.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### tdetelnetservice ############################

set( target tdetelnetservice )

tde_add_executable( ${target}
  SOURCES ${target}.cpp
  LINK tdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

tde_install_symlink( ${target} ${BIN_INSTALL_DIR}/filesharelist )
