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

add_subdirectory( file )
add_subdirectory( http )
add_subdirectory( ftp )
add_subdirectory( gzip )
add_subdirectory( bzip2 )
tde_conditional_add_subdirectory( WITH_LZMA xz )
add_subdirectory( metainfo )
add_subdirectory( iso )
