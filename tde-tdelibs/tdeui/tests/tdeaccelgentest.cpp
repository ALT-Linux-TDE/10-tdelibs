#include "tdeaccelgen.h"

#include <tqstringlist.h>

#include <iostream>

using std::cout;
using std::endl;

void check( const TQString &what, const TQStringList &expected, const TQStringList &received )
{
    cout << "Testing " << what.latin1() << ": ";
    if ( expected == received ) {
        cout << "ok" << endl;
    } else {
        cout << "ERROR!" << endl;
        cout << "Expected: " << expected.join( "," ).latin1() << endl;
        cout << "Received: " << received.join( "," ).latin1() << endl;
    }
}

int main()
{
    TQStringList input;
    input << "foo" << "bar item" << "&baz" << "bif" << "boz" << "boz 2"
	      << "yoyo && dyne";

    TQStringList expected;
    expected << "&foo" << "bar &item" << "&baz" << "bif" << "b&oz" << "boz &2"
	         << "&yoyo && dyne";

    TQStringList output;
    TDEAccelGen::generate( input, output );
    check( "TQStringList value generation", expected, output );

    TQMap<TQString,TQString> map;
    for (TQStringList::ConstIterator it = input.begin(); it != input.end(); ++it) {
        map.insert(*it, *it);
    }
    input.sort();
    expected.clear();
    TDEAccelGen::generate( input, expected );

    output.clear();
    TDEAccelGen::generateFromValues( map, output );
    check( "map value generation", expected, output );

    output.clear();
    TDEAccelGen::generateFromKeys( map, output );
    check( "map key generation", expected, output );
}
