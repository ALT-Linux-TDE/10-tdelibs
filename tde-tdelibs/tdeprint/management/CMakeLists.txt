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

add_definitions(
  -D_TDEPRINT_COMPILE
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/tdeprint
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kmmainview.h kmjobviewer.h kmprinterpage.h
    kiconselectaction.h kmtimer.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeprint )


##### other data ################################

install( FILES
    tileup.png tiledown.png side.png
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint )


#### tdeprint_management ########################

set( target tdeprint_management )

set( ${target}_SRCS
  kmdbcreator.cpp kmdriverdb.cpp kmdriverdbwidget.cpp
  kmiconview.cpp kminfopage.cpp kmmainview.cpp kmpages.cpp
  kmprinterview.cpp kmjobviewer.cpp jobitem.cpp kmpropwidget.cpp
  kmpropcontainer.cpp kmpropertypage.cpp kmpropgeneral.cpp
  cjanuswidget.cpp kmpropmembers.cpp kmpropbackend.cpp
  kmpropdriver.cpp kmlistview.cpp kminstancepage.cpp
  kmtimer.cpp kmwizard.cpp kmwizardpage.cpp kmwinfopage.cpp
  kmwpassword.cpp kmwsocketutil.cpp kmwbackend.cpp
  kmwsocket.cpp kmwdriver.cpp kmwdriverselect.cpp
  kmwdrivertest.cpp kmwinfobase.cpp kmwname.cpp kmwend.cpp
  kmwclass.cpp kmwlpd.cpp kmwfile.cpp kmdriverdialog.cpp
  kmwsmb.cpp smbview.cpp kmconfigdialog.cpp kmconfigpage.cpp
  kmconfiggeneral.cpp kmspecialprinterdlg.cpp kmwlocal.cpp
  kiconselectaction.cpp kmconfigpreview.cpp sidepixmap.cpp
  pluginaction.cpp kxmlcommanddlg.cpp kxmlcommandselector.cpp
  kmconfigcommand.cpp kmconfigfilter.cpp kmconfigfonts.cpp
  kmconfigjobs.cpp networkscanner.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK tdeprint-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


#### libtdeprint_management_module ##############

set( target libtdeprint_management_module )

set( ${target}_SRCS
  tdeprint_management_module.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeprint_management-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### kaddprinterwizard ##########################

set( target kaddprinterwizard )

set( ${target}_SRCS
  kaddprinterwizard.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeprint-shared
)
