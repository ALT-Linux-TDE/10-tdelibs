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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kpixmap.h kpixmapsplitter.h kpixmapeffect.h
    kimageeffect.h kdrawutil.h tdestyle.h kcpuinfo.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### libtdefx ##################################

set( target tdefx )

set( ${target}_SRCS
  kpixmap.cpp kpixmapsplitter.cpp kpixmapeffect.cpp
  kimageeffect.cpp kdrawutil.cpp tdestyle.cpp kcpuinfo.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK ${TQT_LIBRARIES} ${XRENDER_LIBRARIES} c
  DESTINATION ${LIB_INSTALL_DIR}
)
