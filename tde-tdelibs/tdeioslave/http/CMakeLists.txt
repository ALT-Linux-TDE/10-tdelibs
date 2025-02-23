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

add_subdirectory( kcookiejar )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore/network
  ${CMAKE_SOURCE_DIR}/interfaces
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/httpfilter
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE
    http_cache_cleaner.desktop http.protocol https.protocol
    webdav.protocol webdavs.protocol
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdeioslave-desktops
)


##### tdeio_http_cache_cleaner ####################

set( target tdeio_http_cache_cleaner )

set( ${target}_SRCS
  http_cache_cleaner.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
)


##### tdeio_http ##################################

# FIXME GSSAPI support is not handled yet

set( target tdeio_http )

set( ${target}_SRCS
  http.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK httpfilter-static tdentlm-shared tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
