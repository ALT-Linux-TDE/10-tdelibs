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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

add_definitions(
    -DTQT_NO_CAST_ASCII
    -DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

set( TESTS test1 test2 test3 test4 test5 test6 test7 test8 test9 test_dpointer )

# A couple of test got specific settings
set( test8_KCFG test8a test8b )
set( test_dpointer_MAIN test_dpointer_main.cpp )

foreach( tst ${TESTS} )
  unset( _sources )

  # add main file (the second one is default
  if( ${tst}_MAIN )
    list( APPEND _sources ${${tst}_MAIN} )
  else()
    list( APPEND _sources "${tst}main.cpp" )
  endif()

  # set the default kcfg name
  if( NOT ${tst}_KCFG )
    set( ${tst}_KCFG ${tst} )
  endif()

  # kcfgc's here are not designed to be passed to kde3_add_kcfg_files() macro
  # so we will process them manually
  foreach( kcfg ${${tst}_KCFG})
    add_custom_command(
      OUTPUT  ${kcfg}.cpp ${kcfg}.h
      COMMAND "${CMAKE_BINARY_DIR}/tdecore/tdeconfig_compiler/tdeconfig_compiler"
      ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${kcfg}.kcfg"
        "${CMAKE_CURRENT_SOURCE_DIR}/${kcfg}.kcfgc"
      DEPENDS tdeconfig_compiler "${CMAKE_CURRENT_SOURCE_DIR}/${kcfg}.kcfg"
        "${CMAKE_CURRENT_SOURCE_DIR}/${kcfg}.kcfgc"
    )

    add_test( NAME tdecore/tdeconfig_compiler/${kcfg}-cpp
        COMMAND ${CMAKE_COMMAND} -E compare_files
          "${CMAKE_CURRENT_BINARY_DIR}/${kcfg}.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/${kcfg}.cpp.ref" )

    add_test( NAME tdecore/tdeconfig_compiler/${kcfg}-header
        COMMAND ${CMAKE_COMMAND} -E compare_files
          "${CMAKE_CURRENT_BINARY_DIR}/${kcfg}.h"
          "${CMAKE_CURRENT_SOURCE_DIR}/${kcfg}.h.ref" )

    list( APPEND _sources ${CMAKE_CURRENT_BINARY_DIR}/${kcfg}.cpp )
  endforeach( )

  tde_add_check_executable( ${tst} AUTOMOC
    SOURCES ${_sources}
    LINK tdecore-shared
  )
endforeach( tst ${TESTS} )
