#!/bin/sh

clean_up() {
	rm -f batch.stdout shell.stdout shell.returns batch.returns
	rm -fr test-home
}

clean_up

# create temporary home
export HOME=$PWD/test-home
mkdir $HOME
export ICEAUTHORITY=$HOME/.ICEauthority

echo '* Starting dcop server'
../dcopserver --nofork &
DCOP_SERVER_PID=$!

die() {
    kill $DCOP_SERVER_PID
    [ -n $DCOP_TEST_PID ] && kill $DCOP_TEST_PID
    echo "$1"
    exit 1;
}

trap 'die "The script interrupted by user"' 2 15
while ! ../dcopserver --serverid 2>/dev/null; do
  echo '* Wait for the dcop server'
  sleep 2
done

echo '* Running batch mode'
./dcop_test --batch >batch.stdout || die "Failed to run dcop_test"

echo -n '* Starting test app '
./dcop_test >shell.stdout &
DCOP_TEST_PID=$!

cnt=0
while ! ../client/dcop | grep -q "TestApp-$DCOP_TEST_PID"; do
    echo -n '.'
    cnt=$((cnt+1))
    if [ "$cnt" -gt 15 ] ; then
        kill "$DCOP_TEST_PID"
        die "dcop_test seems to hanged up"
    fi
    kill -0 "$DCOP_TEST_PID" || die "dcop_test died unexpectadly"
    sleep 1
done

echo ' started'

echo '* Running driver mode'
./driver "TestApp-$DCOP_TEST_PID" >driver.stdout || die "Failed to start driver"

echo '* Running shell mode'
. ./shell.generated >shell.returns

echo -n '* Comparing ... '

compare()
{
if ! diff -q --strip-trailing-cr $1 $2; then
	echo "FAILED:"
	diff -u $1 $2
	die "$1 and $2 are different";
fi
}

compare batch.stdout shell.stdout
compare batch.stdout driver.stdout
compare batch.returns shell.returns
compare batch.returns driver.returns

clean_up

kill $DCOP_SERVER_PID
echo "Passed"
exit 0;
