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
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES kdedmodule.h DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

tde_create_translated_desktop(
  SOURCE kdedmodule.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
install( FILES tde-applications.menu tde-applications.menu-no-kde DESTINATION ${XDG_MENU_INSTALL_DIR} )
install( FILES kded.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


#### kded #######################################

set( target kded )

set( ${target}_SRCS
  kded.cpp kdedmodule.cpp
)

# we must to export tdeinit_kded-shared because various
# modules need this library
tde_add_tdeinit_executable( ${target} AUTOMOC EXPORT
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
)


#### tdebuildsycoca ###############################

set( target tdebuildsycoca )

set( ${target}_SRCS
  tdebuildsycoca.cpp kbuildservicetypefactory.cpp
  kbuildservicefactory.cpp kbuildservicegroupfactory.cpp
  kbuildimageiofactory.cpp kbuildprotocolinfofactory.cpp
  kctimefactory.cpp vfolder_menu.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
)


#### kdontchangethehostname #####################

set( target kdontchangethehostname )

set( ${target}_SRCS
  khostname.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tde-menu ###################################

set( target tde-menu )

set( ${target}_SRCS
  tde-menu.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdemimelist ##################################

set( target tdemimelist )

set( ${target}_SRCS
  tdemimelist.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
