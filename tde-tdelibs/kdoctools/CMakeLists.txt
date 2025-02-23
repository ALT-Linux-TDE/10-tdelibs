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

include( ConfigureChecks.cmake )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeioslave
  ${LIBXML2_INCLUDE_DIRS}
  ${LIBXSLT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/checkXML DESTINATION ${BIN_INSTALL_DIR} )
install( PROGRAMS move_tdeio_help_cache.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES tdeio_help.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES help.protocol ghelp.protocol DESTINATION ${SERVICES_INSTALL_DIR} )

file(
  COPY
    customization docbook
    man-template.docbook template.docbook
  DESTINATION
    ${CMAKE_BINARY_DIR}/ksgmltools2
  PATTERN ".svn" EXCLUDE
  REGEX "customization/entities/general.entities$" EXCLUDE
)
configure_file(
  customization/entities/general.entities
  ${CMAKE_BINARY_DIR}/ksgmltools2/customization/entities/general.entities
  @ONLY
)

install(
  DIRECTORY
    ${CMAKE_BINARY_DIR}/ksgmltools2/
  DESTINATION
    ${DATA_INSTALL_DIR}/ksgmltools2
)


##### kbzipfilter_dummy ##############################

tde_add_library( kbzipfilter_dummy STATIC_PIC
  SOURCES ${CMAKE_SOURCE_DIR}/tdeioslave/bzip2/kbzip2filter.cpp
)


#### tdeio_help ###################################

set( target tdeio_help )

set( ${target}_SRCS
  tdeio_help.cpp main.cpp xslt.cpp
)

tde_add_kpart( ${target} SHARED
  SOURCES ${${target}_SRCS}
  LINK kbzipfilter_dummy-static tdeio-shared ${BZIP2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### tdeio_ghelp ###################################

set( target tdeio_ghelp )

set( ${target}_SRCS
  tdeio_help.cpp main_ghelp.cpp xslt.cpp
)

tde_add_kpart( ${target} SHARED
  SOURCES ${${target}_SRCS}
  LINK kbzipfilter_dummy-static tdeio-shared ${BZIP2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### meinproc ###################################

set( target meinproc )

set( ${target}_SRCS
  meinproc.cpp xslt_pure.cpp
)

# FIXME I think this can be improved
add_custom_command( OUTPUT xslt_pure.cpp COMMAND cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xslt.cpp xslt_pure.cpp )
set_source_files_properties( xslt_pure.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kbzipfilter_dummy-static tdeio-shared ${BZIP2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
