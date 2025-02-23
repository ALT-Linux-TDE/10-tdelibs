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

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WOVERLOADED_VIRTUAL}" )

add_subdirectory( misc )
add_subdirectory( dom )
add_subdirectory( css )
add_subdirectory( xml )
add_subdirectory( html )
add_subdirectory( rendering )
add_subdirectory( java )
add_subdirectory( ecma )
add_subdirectory( tdemultipart )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kjs
  ${CMAKE_BINARY_DIR}/tdeio/kssl
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/tdeio/kssl
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/tdeprint
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    tdehtmlview.h tdehtml_part.h tdehtml_events.h
    tdehtml_settings.h tdehtmldefaults.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdehtml.desktop tdehtmlimage.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdehtml-desktops
)

install( FILES tdehtml.rc tdehtml_browser.rc tdehtml_popupmenu.rc DESTINATION ${DATA_INSTALL_DIR}/tdehtml )


##### tdehtml #####################################

set( target tdehtml )

set( ${target}_SRCS
  tdehtmlview.cpp tdehtml_part.cpp tdehtml_run.cpp
  tdehtml_factory.cpp tdehtml_settings.cpp tdehtml_events.cpp
  tdehtml_ext.cpp tdehtml_pagecache.cpp tdehtml_iface.cpp
  tdehtml_printsettings.cpp tdehtmlimage.cpp kjserrordlg.ui
  htmlpageinfo.ui tdehtml_iface.skel
)

### FIXME what is this?
### if include_VERSION_SCRIPT
### VSCRIPT = -Wl,--version-script=$(srcdir)/libtdehtml.map
### endif

### FIXME what about java?
### FIXME maybe static libs must be embedded?

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK ${VSCRIPT} kjava-static kjs_html-static tdehtmldom-static tdehtmlcss-static
    tdehtmlhtml-static tdehtmlxml-static tdehtmlrender-static tdehtmlmisc-static
    tdeutils-shared tdeprint-shared
  LINK_PRIVATE kjs-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### libtdehtmlpart ##############################

set( target libtdehtmlpart )

set( ${target}_SRCS
  dummy.cpp
)

file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
"#include <tdelibs_export.h>
extern \"C\" TDE_EXPORT void* init_libtdehtml();
extern \"C\" TDE_EXPORT void* init_libtdehtmlpart() { return init_libtdehtml(); }
" )

tde_add_kpart( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdehtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### tdehtmlimagepart ############################

set( target tdehtmlimagepart )

set( ${target}_SRCS
  imgdummy.cpp
)

file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/imgdummy.cpp
"
#include <tdelibs_export.h>
extern \"C\"  TDE_EXPORT void* init_tdehtmlimagefactory();
extern \"C\"  TDE_EXPORT void* init_tdehtmlimagepart() { return init_tdehtmlimagefactory(); }
" )

tde_add_kpart( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdehtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
