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
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeprint
  ${CMAKE_SOURCE_DIR}/tdeprint/management
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    printcapentry.h lprhandler.h lprsettings.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeprint/lpr )


##### other data ################################

tde_create_translated_desktop(
  SOURCE lpr.print
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/plugins
  PO_DIR tdeprint-desktops
)

install( FILES apsdriver1 apsdriver2 lprngtooldriver1 DESTINATION ${DATA_INSTALL_DIR}/tdeprint )


#### tdeprint_lpr ###############################

set( target tdeprint_lpr )

set( ${target}_SRCS
  printcapreader.cpp lprhandler.cpp matichandler.cpp
  klprfactory.cpp kmlprmanager.cpp kmlpruimanager.cpp
  lpchelper.cpp lpqhelper.cpp kmlprjobmanager.cpp
  matichelper.cpp klprprinterimpl.cpp printcapentry.cpp
  lprsettings.cpp kmconfiglpr.cpp apshandler.cpp
  lprngtoolhandler.cpp editentrydialog.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeprint_management-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### make_driver_db_lpr #########################

set( target make_driver_db_lpr )

set( ${target}_SRCS
  make_driver_db_lpr.c
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK driverparse-static tdefakes-shared ${DL_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
