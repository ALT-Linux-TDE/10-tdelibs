#!/bin/sh

# create temporary home
HOME=@CMAKE_CURRENT_BINARY_DIR@/test-home
mkdir $HOME $HOME/.trinity $HOME/.trinity/share
ln -s @CMAKE_SOURCE_DIR@/mimetypes $HOME/.trinity/share/mimelnk
export HOME
export ICEAUTHORITY=$HOME/.ICEauthority

# run dcop server
../../dcop/dcopserver --nofork &
DCOP_SERVER_PID=$!

while ! ../../dcop/dcopserver --serverid 2>/dev/null; do
  sleep 2
done

# update tdesycoca using mime types from source
../../kded/tdebuildsycoca --noincremental --nosignal

# perform test
./kmimetypetest
STATE=$?

# cleanup
kill $DCOP_SERVER_PID
rm -fr test-home
exit $STATE
