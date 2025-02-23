#!/bin/sh

# create temporary home
export HOME=$PWD/test-home
mkdir $HOME
export ICEAUTHORITY=$HOME/.ICEauthority

# run dcop server
../../../dcop/dcopserver --nofork &
DCOP_SERVER_PID=$!

while ! ../../../dcop/dcopserver --serverid 2>/dev/null; do
  sleep 2
done

# perform test
./backendtest
STATE=$?

# cleanup
kill $DCOP_SERVER_PID
rm -fr test-home
exit $STATE
