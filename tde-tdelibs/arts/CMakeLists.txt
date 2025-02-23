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

if( ARTS_FOUND )
  add_subdirectory( kde )
  add_subdirectory( message )
endif( ARTS_FOUND )

add_subdirectory( knotify )
