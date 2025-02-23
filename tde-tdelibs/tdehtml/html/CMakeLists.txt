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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdehtml
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/tdeio/kssl
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_SOURCE_DIR}/tdewallet/client
)

##### tdehtmlhtml-static ###########################

set( target tdehtmlhtml )

set( ${target}_SRCS
  htmlparser.cpp htmltokenizer.cpp dtd.cpp html_headimpl.cpp
  html_blockimpl.cpp html_elementimpl.cpp html_inlineimpl.cpp
  html_documentimpl.cpp html_baseimpl.cpp html_imageimpl.cpp
  html_listimpl.cpp html_miscimpl.cpp html_formimpl.cpp
  html_objectimpl.cpp html_tableimpl.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
