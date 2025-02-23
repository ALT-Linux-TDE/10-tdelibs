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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES
    kde_logo.png cups_logo.png cupsd.conf.template
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint )


#### cupsdconf ##################################

set( target cupsdconf )

set( ${target}_SRCS
  cupsdconf.cpp cupsddialog.cpp cupsdpage.cpp cupsdcomment.cpp
  cupsdsplash.cpp cupsdserverpage.cpp cupsdlogpage.cpp
  cupsdjobspage.cpp cupsdfilterpage.cpp qdirlineedit.cpp
  cupsddirpage.cpp portdialog.cpp cupsdnetworkpage.cpp
  editlist.cpp cupsdbrowsingpage.cpp browsedialog.cpp
  cupsdsecuritypage.cpp locationdialog.cpp addressdialog.cpp
  cups-util.c qdirmultilineedit.cpp sizewidget.cpp main.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared ${CUPS_LIBRARIES}
)
