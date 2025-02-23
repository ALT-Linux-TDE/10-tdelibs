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
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_BINARY_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${LIBR_INCLUDEDIR}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBR_LIBDIR}
)

#### other data #################################

tde_create_translated_desktop(
  SOURCE tdefile_elf.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdefile-desktops
)


#### tdefile_elf (module) #########################

tde_add_kpart( tdefile_elf AUTOMOC
  SOURCES tdefile_elf.cpp
  EMBED tdelficon-static
  LINK tdeio-shared ${LIBR_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
