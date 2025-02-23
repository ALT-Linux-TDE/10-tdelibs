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

add_subdirectory( management )
add_subdirectory( lpdunix )
if( CUPS_FOUND )
  add_subdirectory( cups )
endif( CUPS_FOUND )
add_subdirectory( lpr )
add_subdirectory( rlpr )
add_subdirectory( ext )
add_subdirectory( filters )
add_subdirectory( pics )
add_subdirectory( tools )

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
  ${CMAKE_SOURCE_DIR}/kded
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kprinter.h kprintaction.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )

install( FILES
    kprintdialogpage.h kpreloadobject.h kmmanager.h
    kmprinter.h kmobject.h kprintdialog.h driver.h
    kmjobmanager.h kmjob.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeprint )


##### other data ################################

install( FILES kprintpreviewui.rc DESTINATION ${CONFIG_INSTALL_DIR}/ui )

install( FILES testprint.ps DESTINATION ${DATA_INSTALL_DIR}/tdeprint )

tde_create_translated_desktop(
  SOURCE specials.desktop
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint
  PO_DIR tdeprint-desktops
)

tde_create_translated_desktop(
  SOURCE tdeprintd.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
  PO_DIR tdeprint-desktops
)
install( FILES tdeprintrc DESTINATION ${CONFIG_INSTALL_DIR} )

install( FILES tdeprintrc.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### driverparse ###############################

set( target driverparse )

tde_add_library( ${target} STATIC_PIC
  SOURCES driverparse.c
)


##### kded_tdeprintd ############################

set( target kded_tdeprintd )

set( ${target}_SRCS
  tdeprintd.cpp kprintprocess.cpp tdeprintd.skel
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared tdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### tdeprint ###################################

set( target tdeprint )

set( ${target}_SRCS
  kmfactory.cpp kmjob.cpp kmjobmanager.cpp kmmanager.cpp
  kmprinter.cpp kmuimanager.cpp kmvirtualmanager.cpp
  driver.cpp driveritem.cpp droptionview.cpp driverview.cpp
  kmdbentry.cpp kprintpreview.cpp kprintdialog.cpp
  kprinter.cpp kprinterimpl.cpp kprintdialogpage.cpp
  kpcopiespage.cpp kpdriverpage.cpp kprinterpropertydialog.cpp
  kpgeneralpage.cpp kpqtpage.cpp kmthreadjob.cpp
  kpipeprocess.cpp kpfilterpage.cpp kmspecialmanager.cpp
  tdeprintcheck.cpp plugincombobox.cpp kpreloadobject.cpp
  tdefilelist.cpp kprintaction.cpp kpfileselectpage.cpp
  treecombobox.cpp kxmlcommand.cpp util.cpp printerfilter.cpp
  marginpreview.cpp marginvaluewidget.cpp marginwidget.cpp
  kpmarginpage.cpp messagewindow.cpp posterpreview.cpp
  kpposterpage.cpp foomatic2loader.cpp fooparser.cpp
  fooscanner.cpp ppdparser.cpp ppdscanner.cpp ppdloader.cpp
  kmfactory.skel
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK tdeio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)

set_target_properties( ${target}-shared PROPERTIES COMPILE_FLAGS -D_TDEPRINT_COMPILE )
