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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### tdetradertest ###############################

set( target tdetradertest )

set( ${target}_SRCS
  tdetradertest.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### check and test ##############################

# Force assertions to work
add_definitions( -UNDEBUG )

set( CHECKS
  getalltest jobtest kdcopcheck kdefaultprogresstest kdirlistertest
  kdirwatchtest kfiltertest kionetrctest kiopassdlgtest kmfitest kmimefromext
  kmimemagictest kpropsdlgtest kprotocolinfotest kruntest kscantest kshredtest
  ktartest kurifiltertest kurlcompletiontest kziptest metatest netaccesstest
  previewtest speed tdeioslavetest tdesycocatest tdesycocaupdatetest
  kmimetypetest
)

foreach( _check ${CHECKS} )
  tde_add_check_executable( ${_check} AUTOMOC LINK tdeio-shared )
endforeach( )

# NOTE some apps here are suitable to be run as tests but not included due to
#      following reasons:
#  - jobtest may fail if run on an unusual system configuration
#            and requires GUI
#  - kprotocolinfotest tries to connect to network and requires running DCOP
#  - kurifiltertest requires network access and GUI
#  - tdesycocatest requires installed tdebase and GUI

set( TESTS dataprotocoltest kacltest )

foreach( _check ${TESTS} )
    tde_add_check_executable( ${_check} AUTOMOC LINK tdeio-shared TEST)
endforeach( )


configure_file( kmimetypetest.sh.cmake kmimetypetest.sh @ONLY )

add_test( NAME "tdeio/kmimetypetest"
    COMMAND sh -x "${CMAKE_CURRENT_BINARY_DIR}/kmimetypetest.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" )
set_tests_properties( "tdeio/kmimetypetest" PROPERTIES TIMEOUT 30)


# TODO add kdirwatchunittest
