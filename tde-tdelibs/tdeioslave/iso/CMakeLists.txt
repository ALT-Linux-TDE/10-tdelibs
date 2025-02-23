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

add_subdirectory( libisofs )


include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE iso.protocol
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdeioslave-desktops
)

tde_create_translated_desktop(
  SOURCE isoservice.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus
  PO_DIR tdeioslave-desktops
)

tde_create_translated_desktop(
  SOURCE tdeio_isorc
  DESTINATION ${CONFIG_INSTALL_DIR}
  PO_DIR tdeioslave-desktops
)

tde_create_translated_desktop(
  SOURCE tdeio_iso.desktop
  DESTINATION ${APPS_INSTALL_DIR}
  PO_DIR tdeioslave-desktops
)


##### tdeio_iso ###################################

set( target tdeio_iso )

set( ${target}_SRCS
  kisodirectory.cpp kisofile.cpp qfilehack.cpp
  kiso.cpp iso.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK isofs-static tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

##### tdeiso_info #################################

set( target tdeiso_info )

set( ${target}_SRCS
  kisodirectory.cpp kisofile.cpp qfilehack.cpp
  kiso.cpp tdeiso_info.cpp
)

tde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK isofs-static tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)