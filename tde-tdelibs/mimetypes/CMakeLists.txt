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

add_subdirectory( application )
add_subdirectory( text )
add_subdirectory( audio )
add_subdirectory( video )
add_subdirectory( image )
add_subdirectory( inode )
add_subdirectory( message )
add_subdirectory( multipart )
add_subdirectory( model )
add_subdirectory( uri )

##### other data ################################

tde_create_translated_desktop(
  SOURCE all.desktop allfiles.desktop
  DESTINATION ${MIME_INSTALL_DIR}/all
  PO_DIR mimetypes
)
