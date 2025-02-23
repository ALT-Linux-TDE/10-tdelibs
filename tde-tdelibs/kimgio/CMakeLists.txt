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

set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${USE_EXCEPTIONS} )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### kimg_eps ##################################

set( target kimg_eps )

tde_add_kpart( ${target}
  SOURCES eps.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_xview ################################

set( target kimg_xview )

tde_add_kpart( ${target}
  SOURCES xview.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_tiff #################################

if( TIFF_FOUND )
  set( target kimg_tiff )
  tde_add_kpart( ${target}
    SOURCES tiffr.cpp
    LINK tdecore-shared ${TIFF_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
  tde_create_translated_desktop(
    SOURCE tiff.kimgio
    DESTINATION ${SERVICES_INSTALL_DIR}
    PO_DIR mimetypes
  )
endif( TIFF_FOUND )


##### kimg_ico ##################################

set( target kimg_ico )

tde_add_kpart( ${target}
  SOURCES ico.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_jp2 ##################################

if( JASPER_FOUND )
  set( target kimg_jp2 )
  tde_add_kpart( ${target}
    SOURCES jp2.cpp
    LINK tdecore-shared ${JASPER_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
  tde_create_translated_desktop(
    SOURCE jp2.kimgio
    DESTINATION ${SERVICES_INSTALL_DIR}
    PO_DIR mimetypes
  )
endif( JASPER_FOUND )


##### kimg_pcx ##################################

set( target kimg_pcx )
tde_add_kpart( ${target}
  SOURCES pcx.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_tga ##################################

set( target kimg_tga )
tde_add_kpart( ${target}
  SOURCES tga.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_rgb ##################################

set( target kimg_rgb )
tde_add_kpart( ${target}
  SOURCES rgb.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_xcf ##################################

set( target kimg_xcf )
tde_add_kpart( ${target}
  SOURCES xcf.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_dds ##################################

set( target kimg_dds )
tde_add_kpart( ${target}
  SOURCES dds.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_exr ##################################

if( OPENEXR_FOUND )
  set( target kimg_exr )
  tde_add_kpart( ${target}
    SOURCES exr.cpp
    LINK tdecore-shared ${OPENEXR_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
  # FIXME COMPILE_FLAGS must be written a little better
  string(REGEX REPLACE ";" " -I" OPENEXR_INCLUDE_FLAGS "-I${OPENEXR_INCLUDE_DIRS}")
  set_target_properties( ${target}-module PROPERTIES COMPILE_FLAGS "${OPENEXR_INCLUDE_FLAGS}" )
  tde_create_translated_desktop(
    SOURCE exr.kimgio
    DESTINATION ${SERVICES_INSTALL_DIR}
    PO_DIR mimetypes
  )
endif( OPENEXR_FOUND )


##### kimg_psd ##################################

set( target kimg_psd )
tde_add_kpart( ${target}
  SOURCES psd.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

##### kimg_hdr ##################################

set( target kimg_hdr )
tde_add_kpart( ${target}
  SOURCES hdr.cpp
  LINK tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kimg_webp #################################

if( HAVE_WEBP )
  set( target kimg_webp )
  tde_add_kpart( ${target}
    SOURCES webp.cpp
    LINK tdecore-shared ${WEBP_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
  tde_create_translated_desktop(
    SOURCE webp.kimgio
    DESTINATION ${SERVICES_INSTALL_DIR}
    PO_DIR mimetypes
  )
endif( HAVE_WEBP )


##### other data ################################

tde_create_translated_desktop(
  SOURCE
    png.kimgio xpm.kimgio bmp.kimgio pbm.kimgio pgm.kimgio
    ppm.kimgio xbm.kimgio jpeg.kimgio xv.kimgio eps.kimgio
    ico.kimgio gif.kimgio pcx.kimgio tga.kimgio rgb.kimgio
    xcf.kimgio dds.kimgio mng.kimgio psd.kimgio hdr.kimgio
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR mimetypes
)
