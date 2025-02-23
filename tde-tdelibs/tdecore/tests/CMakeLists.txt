#################################################
#
#  (C) 2016 Alexander Golubev
#  fatzer2 (AT) gmail.com
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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)

tde_add_library( tdeconfigtest SHARED AUTOMOC
  SOURCES tdeconfigtest.cpp
  LINK tdeunittest-shared
  EXCLUDE_FROM_ALL
)

set( CHECKS
  tdeconfigtestgui klocaletest kprocesstest ksimpleconfigtest kstddirstest
  kuniqueapptest ktempfiletest krandomsequencetest kdebugtest
  ksocktest kstringhandlertest kcmdlineargstest kapptest kmemtest
  dcopkonqtest kipctest cplusplustest kiconloadertest kresolvertest
  kmdcodectest knotifytest ksortablevaluelisttest krfcdatetest testqtargs
  kprociotest kcharsetstest kcalendartest kmacroexpandertest kshelltest
  kxerrorhandlertest startserviceby kglobaltest ktimezonestest
)


foreach( _check ${CHECKS} )
  tde_add_check_executable( ${_check} AUTOMOC LINK tdeconfigtest-shared )
endforeach( )

tde_add_check_executable( kurltest        AUTOMOC LINK tdeconfigtest-shared TEST )
tde_add_check_executable( tdestdacceltest AUTOMOC LINK tdeconfigtest-shared TEST )

tde_add_check_executable( kidlservertest AUTOMOC SOURCES KIDLTest.cpp KIDLTest.skel
  LINK tdeconfigtest-shared )
tde_add_check_executable( kidlclienttest AUTOMOC SOURCES KIDLTestClient.cpp
  KIDLTest.stub LINK tdeconfigtest-shared )

set_property(
  SOURCE KIDLTestClient.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/KIDLTest_stub.cpp
)
