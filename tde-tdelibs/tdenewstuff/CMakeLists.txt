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
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    downloaddialog.h engine.h entry.h knewstuffgeneric.h
    knewstuff.h providerdialog.h provider.h uploaddialog.h
    knewstuffbutton.h knewstuffsecure.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdenewstuff )


##### other data ################################

install( FILES types DESTINATION ${DATA_INSTALL_DIR}/tdenewstuff )
install( FILES tdehotnewstuffrc DESTINATION ${CONFIG_INSTALL_DIR} )
tde_install_icons( )


##### tdenewstuff ###############################

set( target tdenewstuff )

set( ${target}_SRCS
  engine.cpp entry.cpp downloaddialog.cpp uploaddialog.cpp
  providerdialog.cpp provider.cpp knewstuff.cpp security.cpp
  knewstuffgeneric.cpp knewstuffbutton.cpp knewstuffsecure.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### tdehotnewstuff ##############################

set( target tdehotnewstuff )

set( ${target}_SRCS
  tdehotnewstuff.cpp
)

tde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdenewstuff-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
