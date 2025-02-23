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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${LIBPCRE2_INCLUDEDIR}
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
  ${LIBPCRE2_LIBDIR}
)


##### headers ###################################

install( FILES
    value.h types.h object.h interpreter.h operations.h
    ustring.h lookup.h ${CMAKE_CURRENT_BINARY_DIR}/global.h
    identifier.h property_map.h reference.h reference_list.h
    completion.h scope_chain.h list.h simple_number.h function.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kjs )


##### libkjs ####################################

set( target kjs )

set( ${target}_SRCS
  ustring.cpp date_object.cpp collector.cpp nodes.cpp
  grammar.cpp lexer.cpp lookup.cpp operations.cpp
  regexp.cpp function_object.cpp string_object.cpp
  bool_object.cpp number_object.cpp internal.cpp
  array_object.cpp math_object.cpp object_object.cpp
  regexp_object.cpp error_object.cpp function.cpp
  debugger.cpp value.cpp list.cpp object.cpp
  interpreter.cpp property_map.cpp nodes2string.cpp
  identifier.cpp reference.cpp reference_list.cpp
  scope_chain.cpp dtoa.cpp
)

tde_add_luts(
  array_object.cpp math_object.cpp date_object.cpp
  number_object.cpp string_object.cpp
)

tde_add_lut( keywords.table lexer.lut.h lexer.cpp )

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 1.2.0
  LINK tdecore-shared
  LINK_PRIVATE ${LIBPCRE2_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
