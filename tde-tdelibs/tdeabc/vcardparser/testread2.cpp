#include "testutils.h"
#include <tdeabc/addressee.h>
#include <vcardconverter.h>
#include <kdebug.h>

using namespace TDEABC;

int
main()
{
    int rc=0;
    Addressee::List l = vCardsAsAddresseeList();
    TQString vcards = vCardsAsText();

    VCardConverter vct;

    Addressee::List parsed = vct.parseVCards( vcards );

    if ( l.size() != parsed.size() ) {
        kdDebug()<<"\tSize - FAILED : "<<l.size()<<" vs. parsed "<<parsed.size()<<endl;
        rc=1;
    } else {
        kdDebug()<<"\tSize - PASSED"<<endl;
    }

    Addressee::List::iterator itr1;
    Addressee::List::iterator itr2;
    for ( itr1 = l.begin(), itr2 = parsed.begin();
          itr1 != l.end(); ++itr1, ++itr2 ) {
        if ( (*itr1).fullEmail() == (*itr2).fullEmail() &&
             (*itr1).organization() == (*itr2).organization() &&
             (*itr1).phoneNumbers() == (*itr2).phoneNumbers()  &&
             (*itr1).emails() == (*itr2).emails() &&
             (*itr1).role() == (*itr2).role()  ) {
            kdDebug()<<"\tAddressee  - PASSED"<<endl;
            kdDebug()<<"\t\t"<< (*itr1).fullEmail() << " VS. " << (*itr2).fullEmail()<<endl;
        } else {
            kdDebug()<<"\tAddressee  - FAILED"<<endl;
            kdDebug()<<">>>>>>>Addressee from code<<<<<<<<"<<endl;
            (*itr1).dump();
            kdDebug()<<">>>>>>>Addressee from file<<<<<<<<"<<endl;
            (*itr2).dump();
            //kdDebug()<<"\t\t"<< (*itr1).fullEmail() << " VS. " << (*itr2).fullEmail()<<endl;
            rc=1;
        }
    }

    return rc;
}
