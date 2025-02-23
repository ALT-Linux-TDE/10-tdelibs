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
  ${CMAKE_BINARY_DIR}
)


##### isofs-static ##############################

set( target isofs )

tde_add_library( ${target} STATIC_PIC
  SOURCES isofs.c
)
