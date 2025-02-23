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
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/tdeutils
)


##### other data ################################

install( FILES html4.css quirks.css DESTINATION ${DATA_INSTALL_DIR}/tdehtml/css )


##### tdehtmlcss (static) #########################

set( target tdehtmlcss )

set( ${target}_SRCS
  css_stylesheetimpl.cpp css_ruleimpl.cpp css_valueimpl.cpp
  css_base.cpp cssparser.cpp cssstyleselector.cpp
  csshelper.cpp parser.cpp css_renderstyledeclarationimpl.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)

