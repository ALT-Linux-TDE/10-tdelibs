#################################################
#
#  (C) 2012-2020 Trinity Project
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

if( NOT TDE_RELEASE_ENTITIES )

  set( TDEVERSION_FILE "${CMAKE_SOURCE_DIR}/tdecore/tdeversion.h" )
  set( ENTITIES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/customization/entities/general.entities" )

  if( NOT EXISTS ${TDEVERSION_FILE} )
    file( RELATIVE_PATH TDEVERSION_FILE ${CMAKE_SOURCE_DIR} ${TDEVERSION_FILE} )
    tde_message_fatal( "${TDEVERSION_FILE} not found! Check your sources." )
  endif( )
  if( NOT EXISTS ${ENTITIES_FILE} )
    file( RELATIVE_PATH ENTITIES_FILE ${CMAKE_SOURCE_DIR} ${ENTITIES_FILE} )
    tde_message_fatal( "${ENTITIES_FILE} not found! Check your sources." )
  endif( )

  # read source metadata
  tde_read_src_metadata()

  # read TDE_VERSION_STRING
  file( STRINGS ${TDEVERSION_FILE} TDE_VERSION_STRING REGEX "define TDE_VERSION_STRING" )
  string( REGEX REPLACE ".*#define TDE_VERSION_STRING \"([^\"]*)\".*" "\\1"
          TDE_VERSION_STRING "${TDE_VERSION_STRING}" )
  if( "${TDE_VERSION_STRING}" STREQUAL "" )
    tde_message_fatal( "Cannot determine the Trinity version number." )
  endif( )

  # compose TDE_RELEASE_DATE
  if( "${TDE_VERSION_STRING}" MATCHES "DEVELOPMENT" )
    tde_curdatetime( TDE_RELEASE_DATE )
  else( )
    if( TDE_SCM_MODULE_DATETIME )
      set( TDE_RELEASE_DATE "${TDE_SCM_MODULE_DATETIME}" )
    else( )
      if( ${CMAKE_SYSTEM_NAME} MATCHES "BSD" )
        set( GNU_FIND_EXECUTABLE "gfind" )
      else( )
        set( GNU_FIND_EXECUTABLE "find" )
      endif( )
      execute_process(
        COMMAND ${GNU_FIND_EXECUTABLE} ${TDEVERSION_FILE} -printf "%Tm/%Te/%TY"
        OUTPUT_VARIABLE TDE_RELEASE_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    endif( )
  endif( )

  string( REGEX REPLACE "^([0-9]+)/([0-9]+)*/([0-9]+).*" "2010-\\3"
          TDE_RELEASE_COPYRIGHT "${TDE_RELEASE_DATE}" )

  string( REGEX REPLACE "^([0-9]+)/([0-9]+)*/([0-9]+).*" "\\1"
          _release_month_num "${TDE_RELEASE_DATE}" )
  math( EXPR _release_month_index "${_release_month_num}-1" )
  set( _month_names "January;February;March;April;May;June;July;August;September;October;November;December" )
  list( GET _month_names ${_release_month_index} _release_month_name )
  string( REGEX REPLACE
          "^([0-9]+)/([0-9]+)*/([0-9]+).*"
          "${_release_month_name} \\2, \\3"
          TDE_RELEASE_DATE  "${TDE_RELEASE_DATE}" )

  # update entities
  file( RELATIVE_PATH ENTITIES_FILE ${CMAKE_SOURCE_DIR} ${ENTITIES_FILE} )
  message( STATUS "Updating ${ENTITIES_FILE}
    TDE Release Version: ${TDE_VERSION_STRING}
    TDE Release Date: ${TDE_RELEASE_DATE}
    TDE Release Copyright: ${TDE_RELEASE_COPYRIGHT}"
  )
  set( TDE_RELEASE_ENTITIES 1 CACHE INTERNAL "" )

endif( )
