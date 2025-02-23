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
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kjava #####################################

configure_file( ${TDE_CMAKE_TEMPLATES}/tde_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_library( kjava SHARED
  SOURCES dummy.cpp
  LINK ${TQT_LIBRARIES}
  VERSION 1.0.0
  DESTINATION ${LIB_INSTALL_DIR}
)
