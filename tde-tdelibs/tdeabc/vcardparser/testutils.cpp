#include "vcardparser.h"
#include <tdeabc/addressee.h>
#include <tqfile.h>
#include <tqstring.h>

using namespace TDEABC;

Addressee
vcard1()
{
    Addressee addr;

    addr.setName( "Frank Dawson" );
    addr.setOrganization( "Lotus Development Corporation" );
    addr.setUrl( KURL( "http://home.earthlink.net/~fdawson") );
    addr.insertEmail( "fdawson@earthlink.net" );
    addr.insertEmail( "Frank_Dawson@Lotus.com", true );
    addr.insertPhoneNumber( PhoneNumber("+1-919-676-9515",PhoneNumber::Voice|PhoneNumber::Msg
                                        |PhoneNumber::Work ) );
    addr.insertPhoneNumber( PhoneNumber("+1-919-676-9564",PhoneNumber::Fax |PhoneNumber::Work ));
    Address a( Address::Work | Address::Postal | Address::Parcel );
    a.setStreet( "6544 Battleford Drive" );
    a.setLocality( "Raleigh" );
    a.setRegion( "NC" );
    a.setPostalCode( "27613-3502" );
    a.setCountry( "U.S.A." );
    addr.insertAddress( a );
    return addr;
}

Addressee
vcard2()
{
    Addressee addr;

    addr.setName( "Tim Howes" );
    addr.setOrganization( "Netscape Communications Corp." );
    addr.insertEmail( "howes@netscape.com" );
    addr.insertPhoneNumber( PhoneNumber("+1-415-937-3419",PhoneNumber::Voice|PhoneNumber::Msg
                                        |PhoneNumber::Work) );
    addr.insertPhoneNumber( PhoneNumber("+1-415-528-4164",PhoneNumber::Fax |PhoneNumber::Work) );
    Address a( Address::Work );
    a.setStreet( "501 E. Middlefield Rd." );
    a.setLocality( "Mountain View" );
    a.setRegion( "CA" );
    a.setPostalCode( "94043" );
    a.setCountry( "U.S.A." );
    addr.insertAddress( a );
    return addr;
}

Addressee
vcard3()
{
    Addressee addr;

    addr.setName( "ian geiser" );
    addr.setOrganization( "Source eXtreme" );
    addr.insertEmail( "geiseri@yahoo.com" );
    addr.setTitle( "VP of Engineering" );
    return addr;
}

Addressee
vcard8()
{
    Addressee addr;

    addr.setName( TQString::fromUtf8("Jahn") );
    addr.setFamilyName( TQString::fromUtf8("Böhmermann") );
    addr.setFormattedName( TQString::fromUtf8("Jahn Böhmermann") );
    addr.setOrganization( TQString::fromUtf8("HansWürstel AG") );
    addr.insertEmail( TQString::fromUtf8("boehmermann@wuerstel.com") );
    addr.setTitle( TQString::fromUtf8("Komödiant") );
    addr.insertPhoneNumber( PhoneNumber("+43 699373419",PhoneNumber::Voice|PhoneNumber::Msg|PhoneNumber::Work) );
    Address a( Address::Work );
    a.setStreet( TQString::fromUtf8("Müllerstrasse 21") );
    a.setLocality( TQString::fromUtf8("Wörthersee") );
    a.setRegion( TQString::fromUtf8("Kärnten") );
    a.setPostalCode( "8400" );
    a.setCountry( TQString::fromUtf8("Österreich") );
    addr.insertAddress( a );
    return addr;
}

Addressee
vcard9()
{
    Addressee addr;

    addr.setName( TQString::fromUtf8("Иван") );
    addr.setFamilyName( TQString::fromUtf8("Иванов") );
    addr.setFormattedName( TQString::fromUtf8("Иван Иванов") );
    addr.setOrganization( TQString::fromUtf8("България ООД") );
    addr.insertEmail( TQString::fromUtf8("иван.иванов@българия.com") );
    addr.setTitle( TQString::fromUtf8("Др") );
    addr.insertPhoneNumber( PhoneNumber("+359 888 111 222",PhoneNumber::Voice|PhoneNumber::Msg|PhoneNumber::Work) );
    Address a( Address::Work );
    a.setStreet( TQString::fromUtf8("Цар Борис III") );
    a.setLocality( TQString::fromUtf8("София") );
    a.setRegion( TQString::fromUtf8("София град") );
    a.setPostalCode( "1000" );
    a.setCountry( TQString::fromUtf8("България") );
    addr.insertAddress( a );
    return addr;
}


TQString
vcardAsText( const TQString& location )
{
    TQString line;
    TQFile file( location );
    if ( file.open( IO_ReadOnly ) ) {
        TQTextStream stream( &file );
        stream.setEncoding( TQTextStream::UnicodeUTF8 );
        if ( !stream.eof() ) {
            line = stream.read();
        }
        file.close();
    }
    return line;
}

Addressee::List
vCardsAsAddresseeList()
{
    Addressee::List l;

    l.append( vcard1() );
    l.append( vcard2() );
    l.append( vcard3() );
    l.append( vcard8() );
    l.append( vcard9() );

    return l;
}

TQString
 vCardsAsText()
{
    TQString vcards = vcardAsText( "tests/vcard1.vcf" );
    vcards += vcardAsText( "tests/vcard2.vcf" );
    vcards += vcardAsText( "tests/vcard3.vcf" );
    vcards += vcardAsText( "tests/vcard8.vcf" );
    vcards += vcardAsText( "tests/vcard9.vcf" );

    return vcards;
}
