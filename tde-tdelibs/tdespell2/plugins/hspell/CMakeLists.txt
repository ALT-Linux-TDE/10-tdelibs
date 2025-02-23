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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdespell2
  ${CMAKE_SOURCE_DIR}/tdecore
  ${HSPELL_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${HSPELL_LIBARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdespell_hspell.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdespell2-desktops
)


#### tdespell_hspell ##############################

### FIXME Not checked because hspell was removed from gentoo few years ago.
### FIXME The maintainer recommend to use aspell with hebrew dictionary.
### FIXME Maybe support for hspell will be removed too?
### FIXME http://bugs.gentoo.org/show_bug.cgi?id=177546

set( target tdespell_hspell )

set( ${target}_SRCS
  tdespell_hspellclient.cpp tdespell_hspelldict.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdespell2-shared ${HSPELL_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
