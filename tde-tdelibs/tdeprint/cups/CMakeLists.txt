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

add_subdirectory( cupsdconf2 )

add_definitions(
  -D_TDEPRINT_COMPILE
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore/network
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/tdeprint
  ${CMAKE_SOURCE_DIR}/tdeprint/management
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE cups.print
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/plugins
  PO_DIR tdeprint-desktops
)

install( FILES preview.png preview-mini.png DESTINATION ${DATA_INSTALL_DIR}/tdeprint )


#### tdeprint_cups ##############################

set( target tdeprint_cups )

set( ${target}_SRCS
  cupsinfos.cpp ipprequest.cpp kmcupsfactory.cpp
  kmcupsjobmanager.cpp kmcupsmanager.cpp kmcupsuimanager.cpp
  kmpropbanners.cpp kmwbanners.cpp kmwipp.cpp kmwippselect.cpp
  kcupsprinterimpl.cpp kmcupsconfigwidget.cpp kmconfigcups.cpp
  kmconfigcupsdir.cpp kmwippprinter.cpp kmwfax.cpp kmwquota.cpp
  kmpropquota.cpp kmwusers.cpp kmpropusers.cpp kpschedulepage.cpp
  image.cpp imagepreview.cpp imageposition.cpp kpimagepage.cpp
  kptextpage.cpp kphpgl2page.cpp cupsaddsmb2.cpp
  kmwother.cpp ippreportdlg.cpp kptagspage.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeprint_management-shared ${CUPS_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### make_driver_db_cups ########################

set( target make_driver_db_cups )

set( ${target}_SRCS
  make_driver_db_cups.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK driverparse-static tdefakes-shared ${ZLIB_LIBRARIES} ${DL_LIBRARIES} ${TQT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


#### cupsdoprint ################################

set( target cupsdoprint )

set( ${target}_SRCS
  cupsdoprint.c
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdefakes-shared ${CUPS_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
