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

add_subdirectory( tdemdi )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/res
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_SOURCE_DIR}/tdemdi
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    tdemdichildarea.h tdemdichildfrm.h tdemdichildfrmcaption.h
    tdemdidefines.h tdemdiiterator.h tdemdilistiterator.h
    tdemdimainfrm.h tdemdinulliterator.h tdemditaskbar.h
    tdemdichildview.h tdemditoolviewaccessor.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


#### tdemdi #######################################

set( target tdemdi )

set( ${target}_SRCS
  tdemdichildarea.cpp tdemdichildfrm.cpp tdemdichildfrmcaption.cpp
  tdemdichildview.cpp tdemdimainfrm.cpp tdemditaskbar.cpp
  tdemdidockcontainer.cpp tdemditoolviewaccessor.cpp
  tdemdiguiclient.cpp tdemdidocumentviewtabwidget.cpp
  tdemdifocuslist.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdemdi2-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
