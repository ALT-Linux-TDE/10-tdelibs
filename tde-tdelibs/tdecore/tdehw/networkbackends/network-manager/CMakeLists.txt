#################################################
#
#  (C) 2013 Golubev Alexander
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( dbus-introspection )

include_directories (
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DBUS_TQT_INCLUDE_DIRS}
  ${NM_UTIL_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore/tdehw
  ${CMAKE_BINARY_DIR}/tdeio/kssl
  ${CMAKE_CURRENT_BINARY_DIR}/dbus-introspection
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${DBUS_TQT_LIBRARY_DIRS}
)

tde_add_library( network_manager_backend STATIC_PIC AUTOMOC
  SOURCES network-manager.cpp
  EMBED tdenm_dbus-static
  LINK tdenm_dbus-static ${DBUS_TQT_LIBRARIES}
)
