#################################################
#
#  (C) 2012 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing.net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

##### tde_dbus_hardwarecontrol ##################

tde_add_executable( tde_dbus_hardwarecontrol
  SOURCES tde_dbus_hardwarecontrol.c
  LINK ${DBUS_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)

##### other data ################################

configure_file( org.trinitydesktop.hardwarecontrol.service.cmake org.trinitydesktop.hardwarecontrol.service @ONLY )

install( FILES
    org.trinitydesktop.hardwarecontrol.conf
  DESTINATION ${DBUS_SYSTEM_CONF_DIRECTORY} )

install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.trinitydesktop.hardwarecontrol.service
  DESTINATION ${DBUS_SERVICE_DIRECTORY} )
