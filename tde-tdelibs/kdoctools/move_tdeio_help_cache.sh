#!/bin/sh

bas=`tde-config --localprefix`share/apps/tdeio_help
mv "$bas/cache" "`tde-config --path cache`tdeio_help"
rmdir "$bas"
exit 0
