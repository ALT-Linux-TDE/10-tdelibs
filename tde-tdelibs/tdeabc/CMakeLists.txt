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

add_subdirectory( vcard )
add_subdirectory( vcardparser )
add_subdirectory( formats )
add_subdirectory( plugins )
add_subdirectory( tests )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/vcard/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vcard/include/generated
  ${CMAKE_CURRENT_SOURCE_DIR}/vcardparser

  # external includes
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/kab
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    address.h addressbook.h  addresseedialog.h
    agent.h distributionlist.h distributionlistdialog.h
    distributionlisteditor.h errorhandler.h field.h
    format.h formatfactory.h formatplugin.h geo.h key.h
    phonenumber.h picture.h plugin.h resource.h secrecy.h
    resourceselectdialog.h sound.h stdaddressbook.h
    timezone.h vcardconverter.h vcardformat.h lock.h
    vcardformatplugin.h ldifconverter.h addresslineedit.h
    ldapclient.h addresseelist.h locknull.h ldif.h
    ldapurl.h ldapconfigwidget.h sortmode.h
    ${CMAKE_CURRENT_BINARY_DIR}/addressee.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeabc )


##### other data ################################

tde_create_translated_desktop(
  SOURCE tdeab2tdeabc.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR}
  PO_DIR tdeabc-desktops
)

tde_create_translated_desktop(
  SOURCE tdeabc_manager.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/tderesources
  PO_DIR tdeabc-desktops
)

install( FILES countrytransl.map DESTINATION ${DATA_INSTALL_DIR}/tdeabc )


##### generated files ###########################
# FIXME this hack make compatibility with out-of-source mode

file( COPY
    scripts/makeaddressee scripts/addressee.src.cpp
    scripts/addressee.src.h scripts/entrylist scripts/field.src.cpp
  DESTINATION scripts )

add_custom_command(
  OUTPUT addressee.cpp addressee.h field.cpp
  COMMAND perl
  ARGS makeaddressee
  DEPENDS scripts/addressee.src.cpp scripts/addressee.src.h scripts/entrylist scripts/field.src.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts
)

add_custom_target(
  addressee_files
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/addressee.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/addressee.h
    ${CMAKE_CURRENT_BINARY_DIR}/field.cpp
)

##### tdeabc ######################################

set( target tdeabc )

set( ${target}_SRCS
  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp
  agent.cpp distributionlist.cpp distributionlistdialog.cpp
  distributionlisteditor.cpp errorhandler.cpp field.cpp
  formatfactory.cpp geo.cpp key.cpp phonenumber.cpp
  picture.cpp plugin.cpp resource.cpp resourceselectdialog.cpp
  secrecy.cpp sound.cpp stdaddressbook.cpp timezone.cpp
  vcard21parser.cpp vcardconverter.cpp vcardformat.cpp
  vcardformatimpl.cpp vcardformatplugin.cpp ldifconverter.cpp
  addresslineedit.cpp ldapclient.cpp addresseelist.cpp
  vcardtool.cpp addresseehelper.cpp lock.cpp locknull.cpp
  ldif.cpp ldapurl.cpp ldapconfigwidget.cpp sortmode.cpp
  addresseehelper.skel
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.2.0
  LINK vcards-static vcard-shared tdeio-shared tderesources-shared
  DEPENDENCIES addressee_files
  DESTINATION ${LIB_INSTALL_DIR}
)


##### tdeab2tdeabc ##################################

set( target tdeab2tdeabc )

set( ${target}_SRCS
  tdeab2tdeabc.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kab-static tdeabc-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
