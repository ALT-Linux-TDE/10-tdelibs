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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
  ${CMAKE_SOURCE_DIR}/kjs
  ${CMAKE_SOURCE_DIR}/tdeutils
  ${CMAKE_SOURCE_DIR}/tdeprint
  ${CMAKE_SOURCE_DIR}/interfaces
  ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
  ${LUA_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LUA_LIBRARY_DIRS}
)

##### kate (static library) #####################

set( target kate )

set( ${target}_SRCS
  katesearch.cpp katebuffer.cpp katecmds.cpp
  kateundo.cpp katecursor.cpp katedialogs.cpp katedocument.cpp
  katefactory.cpp katehighlight.cpp katesyntaxdocument.cpp
  katetextline.cpp kateview.cpp kateconfig.cpp kateviewhelpers.cpp
  katecodecompletion.cpp katedocumenthelpers.cpp
  katecodefoldinghelpers.cpp kateviewinternal.cpp katebookmarks.cpp
  kateprinter.cpp katefont.cpp katelinerange.cpp katesupercursor.cpp
  katearbitraryhighlight.cpp katerenderer.cpp kateattribute.cpp
  kateautoindent.cpp katefiletype.cpp kateschema.cpp katedocument.skel
  katetemplatehandler.cpp katejscript.cpp katespell.cpp kateindentscriptabstracts.cpp
  kateluaindentscript.cpp
)

tde_add_lut( katejscript.cpp katejscript.lut.h katejscript.cpp )

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)


##### libkatepart (module) ######################

set( target libkatepart )

configure_file( ${TDE_CMAKE_TEMPLATES}/tde_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_kpart( ${target}
  SOURCES dummy.cpp
  EMBED kate-static
  LINK kjs-shared tdeprint-shared katepartinterfaces-shared tdeutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
