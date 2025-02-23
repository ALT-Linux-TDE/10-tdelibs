###########################################
#                                         #
#  Copyright (C)  2022 Mavridis Philippe  #
#                  <mavridisf@gmail.com>  #
#                                         #
#  Improvements and feedback are welcome  #
#                                         #
#  Released under GNU GPL v2 or greater.  #
#                                         #
###########################################

### Header and library directories ########
#
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdehtml
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/md4c/src
)

include_directories( SYSTEM
  ${TQT_INCLUDE_DIRS}
  ${TDE_INCLUDE_DIR}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${TDE_LIB_DIR}
)


### Build libtdemarkdown (kpart) ###########
#
tde_add_kpart( libtdemarkdown
  AUTOMOC
  SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/md4c/src/entity.c
    ${CMAKE_CURRENT_SOURCE_DIR}/md4c/src/md4c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/md4c/src/md4c-html.c
    markdown_part.cpp
  LINK
    tdeparts-shared tdehtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


### Install part .desktop entry #############
#
tde_create_translated_desktop(
  SOURCE markdown_part.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdemarkdown-desktops
)


### Install XML-GUI #########################
#
install(
  FILES markdown_part.rc
  DESTINATION ${DATA_INSTALL_DIR}/tdemarkdown
)
