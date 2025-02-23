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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    defaults.h client.h process.h tdesu_pty.h
    kcookie.h su.h ssh.h stub.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdesu )


##### libtdesu ##################################

set( target tdesu )

set( ${target}_SRCS
  client.cpp process.cpp kcookie.cpp
  su.cpp ssh.cpp stub.cpp tdesu_pty.cpp
)

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 14.0.0
  LINK tdecore-shared
  LINK_PRIVATE ${LIB_UTIL}
  DESTINATION ${LIB_INSTALL_DIR}
)

tde_install_symlink( tdesu_pty.h ${INCLUDE_INSTALL_DIR}/tdesu/pty.h )


##### tdesu_stub ################################

tde_add_executable( tdesu_stub
  SOURCES tdesu_stub.c
  DESTINATION ${BIN_INSTALL_DIR}
)
