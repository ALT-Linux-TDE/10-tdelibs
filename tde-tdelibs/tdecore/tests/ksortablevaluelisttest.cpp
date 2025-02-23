#include <tqstring.h>
#include <ksortablevaluelist.h>

int main( int argc, char **argv )
{
    KSortableValueList<TQString> list;
    list.insert( 1,  "FOO           (1)" );
    list.insert( 2,  "Test          (2)" );
    list.insert( 1,  "Huba!         (1)" );
    list.insert( 5,  "MAAOOAM!      (5)" );
    list.insert( 10, "Teeheeest    (10)" );
    list.insert( 2,  "I was here :) (2)" );
    list.insert( 4,  "Yeehaa...     (4)" );
    
    TQValueListIterator<KSortableItem<TQString> > it = list.begin();

    tqDebug("Insertion order:");
    tqDebug("================");
    for ( ; it != list.end(); ++it )
        tqDebug( "%i: %s", (*it).index(), (*it).value().latin1() );

    list.sort();
    
    tqDebug("\nSorted:");
    tqDebug("=======");
    
    it = list.begin();
    for ( ; it != list.end(); ++it )
        tqDebug( "%i: %s", (*it).index(), (*it).value().latin1() );
    
    return 0;
}
