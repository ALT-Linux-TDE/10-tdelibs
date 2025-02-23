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

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include tqt.h" )

add_definitions(
  -DICE_t
  -DTRANS_SERVER
  -DTRANS_CLIENT
  -DUNIXCONN
  -DANSICPP
)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${TQT_INCLUDE_DIRS}
)


##### kICE ######################################

set( target kICE )

set( ${target}_SRCS
  accept.c authutil.c connect.c error.c getauth.c
  iceauth.c listen.c listenwk.c locking.c misc.c
  ping.c process.c protosetup.c register.c replywait.c
  setauth.c shutdown.c watch.c transport.c globals.c
)

tde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
)
