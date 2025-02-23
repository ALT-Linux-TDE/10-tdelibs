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

add_subdirectory( ksettings )

include_directories(
   ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kfind.h kfinddialog.h kreplace.h kreplacedialog.h
    tdecmoduleinfo.h tdecmoduleloader.h kcmultidialog.h
    tdemultitabbar.h kplugininfo.h tdecmoduleproxy.h
    kpluginselector.h tdecmodulecontainer.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

tde_create_translated_desktop(
  SOURCE kplugininfo.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
)


#### tdeutils #####################################

set( target tdeutils )

set( ${target}_SRCS
  kfind.cpp kfinddialog.cpp kreplace.cpp
  kreplacedialog.cpp tdecmoduleinfo.cpp tdecmoduleloader.cpp
  kcmultidialog.cpp tdemultitabbar.cpp kplugininfo.cpp
  tdecmoduleproxy.cpp kpluginselector.cpp tdecmodulecontainer.cpp
  tdecmoduleproxyIface.skel tdecmoduleproxyIfaceImpl.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.2.0
  EMBED ksettings-static
  LINK tdeparts-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
