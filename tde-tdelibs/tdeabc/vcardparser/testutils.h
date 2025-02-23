#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <tdeabc/addressee.h>
#include <tqstring.h>

TDEABC::Addressee vcard1();
TDEABC::Addressee vcard2();
TDEABC::Addressee vcard3();
TDEABC::Addressee::List vCardsAsAddresseeList();
TQString   vCardAsText( const TQString& location );
TQString   vCardsAsText();

#endif
