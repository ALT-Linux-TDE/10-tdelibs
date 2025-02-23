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

add_subdirectory( kssl )
add_subdirectory( tdeio )
add_subdirectory( bookmarks )
add_subdirectory( tdefile )
add_subdirectory( pics )
add_subdirectory( tdeioexec )
add_subdirectory( httpfilter )
add_subdirectory( misc )
add_subdirectory( kpasswdserver )
add_subdirectory( tests )

include_directories(
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBR_LIBDIR}
)


##### other data ################################

add_custom_command(
  OUTPUT tdeio.magic.mgc
  COMMAND file -C -m ${CMAKE_CURRENT_SOURCE_DIR}/tdeio.magic
)

add_custom_target(
  tdeio_magic_mgc
  ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tdeio.magic.mgc
)

install( FILES tdeio.magic ${CMAKE_CURRENT_BINARY_DIR}/tdeio.magic.mgc DESTINATION ${CONFIG_INSTALL_DIR}/magic )

tde_create_translated_desktop(
  SOURCE
    application.desktop kurifilterplugin.desktop
    kcomprfilter.desktop kscan.desktop kdatatool.desktop
    tdefileplugin.desktop tdecmodule.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR tdeio-desktops
)

install( FILES tdeioslave.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( PROGRAMS useragent.pl proxytype.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

tde_create_translated_desktop(
  SOURCE renamedlgplugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR tdeio-desktops
)

tde_create_translated_desktop(
  SOURCE kpasswdserver.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
  PO_DIR tdeio-desktops
)

tde_create_translated_desktop(
  SOURCE data.protocol
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR tdeio-desktops
)


if( HAVE_ELFICON )
  set( ELFICON_STATIC_LIB tdelficon-static )
endif( HAVE_ELFICON )


##### libtdeio ####################################

set( target tdeio )

configure_file( ${TDE_CMAKE_TEMPLATES}/tde_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_library( ${target} SHARED
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
  VERSION 14.0.0
  EMBED kssl-static tdeiocore-static tdesycoca-static kbookmarks-static tdefile-static ${ELFICON_STATIC_LIB}
  LINK tdeui-shared tdewalletclient-shared
  LINK_PRIVATE ltdlc-static tdesu-shared ${LIBR_LIBRARIES} ${ACL_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
