#! /bin/sh

prefix=@CMAKE_INSTALL_PREFIX@
# FIXME this must be separate?
exec_prefix=@CMAKE_INSTALL_PREFIX@
exit `@BIN_INSTALL_DIR@/meinproc --check --stdout $@ > /dev/null`

