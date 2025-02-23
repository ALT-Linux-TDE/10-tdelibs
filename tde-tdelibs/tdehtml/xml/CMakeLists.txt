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
  ${CMAKE_BINARY_DIR}/kjs
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdehtml
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/tdeutils
)


##### tdehtmlxml-static ############################

set( target tdehtmlxml )

set( ${target}_SRCS
  dom_docimpl.cpp dom_nodeimpl.cpp dom_textimpl.cpp
  dom_elementimpl.cpp dom_stringimpl.cpp dom2_rangeimpl.cpp
  dom2_traversalimpl.cpp xml_tokenizer.cpp dom_xmlimpl.cpp
  dom2_eventsimpl.cpp dom2_viewsimpl.cpp dom_restyler.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
