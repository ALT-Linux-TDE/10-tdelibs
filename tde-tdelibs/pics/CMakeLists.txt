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

add_subdirectory( hicolor )
add_subdirectory( crystalsvg )
add_subdirectory( emoticons )


# ksvgtopng is compiled only if we have libart support
if( WITH_LIBART )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${LIBART_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore/svgicons
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### other data ################################

tde_install_symlink( crystalsvg ${ICON_INSTALL_DIR}/default.tde )


##### ksvgtopng #################################

set( target ksvgtopng )

set( ${target}_SRCS
  ksvgtopng.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kdesvgicons-static tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

endif( WITH_LIBART )
