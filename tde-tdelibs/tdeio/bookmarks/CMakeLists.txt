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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)


##### headers ###################################

install( FILES
    kbookmark.h kbookmarkbar.h kbookmarkdrag.h kbookmarkexporter.h
    kbookmarkimporter.h kbookmarkmanager.h kbookmarkmenu.h
    kbookmarknotifier.h kbookmarkimporter_crash.h
    kbookmarkimporter_opera.h kbookmarkimporter_ie.h
    kbookmarkimporter_ns.h kbookmarkimporter_kde1.h
    kbookmarkdombuilder.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### kbookmarks ################################

set( target kbookmarks )

set( ${target}_SRCS
  kbookmark.cpp kbookmarkbar.cpp kbookmarkdrag.cpp kbookmarkexporter.cpp
  kbookmarkimporter.cpp kbookmarkmanager.cpp kbookmarkmenu.cpp
  kbookmarkimporter_crash.cpp kbookmarkimporter_opera.cpp
  kbookmarkimporter_ie.cpp kbookmarkimporter_ns.cpp
  kbookmarkimporter_kde1.cpp kbookmarkdombuilder.cpp
  kbookmarkmanager.skel kbookmarknotifier.skel
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
