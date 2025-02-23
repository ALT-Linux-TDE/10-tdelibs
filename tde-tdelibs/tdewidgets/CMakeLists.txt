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

add_subdirectory( pics )


add_definitions(
  -DQT_PLUGIN
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### maketdewidgets ############################

set( target maketdewidgets )

set( ${target}_SRCS
  maketdewidgets.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

##### tdewidgets ################################

set( target tdewidgets )

set( ${target}_SRCS
  classpreviews.cpp tdewidgets.cpp
)

add_custom_command( OUTPUT tdewidgets.cpp
  COMMAND ./maketdewidgets -o tdewidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
  DEPENDS maketdewidgets ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeabc-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer
)
