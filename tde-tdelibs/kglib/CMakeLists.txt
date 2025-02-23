#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${GLIB2_LIBRARY_DIRS}
  ${GOBJECT2_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    TGlibEventLoop.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )



#### kglib ######################################

tde_add_library( kglib SHARED
  SOURCES TGlibEventLoop.cpp
  VERSION 0.0.0
  LINK ${TQT_LIBRARIES}
  LINK_PRIVATE ${GLIB2_LIBRARIES} ${GOBJECT2_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
