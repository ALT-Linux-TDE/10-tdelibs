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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE kgzipfilter.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdeioslave-desktops
)


##### kgzipfilter ###############################

set( target kgzipfilter )

set( ${target}_SRCS
  kgzipfilter.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
