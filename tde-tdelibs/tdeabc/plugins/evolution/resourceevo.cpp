#include <tqdir.h>

#include <tdeglobal.h>
#include <tdelocale.h>
#include <kdebug.h>

#include <stdio.h>

#include <tdeabc/vcardparser/vcardtool.h>

#include "dbwrapper.h"
#include "resourceevo.h"

using namespace Evolution;
using namespace TDEABC;

class EvolutionFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const TDEConfig *config )
    {
      return new ResourceEvolution( config );
    }

    KRES::ConfigWidget *configWidget( TQWidget * )
    {
      return 0;
    }
};

extern "C"
{
  TDE_EXPORT void *init_tdeabc_evo()
  {
    return ( new EvolutionFactory() );
  }
}

ResourceEvolution::ResourceEvolution( const TDEConfig* conf )
    : Resource( conf ), mWrap(0l)
{
    m_isOpen = false;
}
ResourceEvolution::~ResourceEvolution() {
    delete mWrap;
}
bool ResourceEvolution::doOpen() {
    mWrap = new DBWrapper;
    if (!mWrap->open( TQDir::homeDirPath() + "/evolution/local/Contacts/addressbook.db" ) ) {
        return false;
    }

    TQString val;
    if (!mWrap->find( "PAS-DB-VERSION", val ) )
        return false;

    if (!val.startsWith("0.2") )
        return false;

    m_isOpen = true;

    return true;
}
void ResourceEvolution::doClose() {
    delete mWrap;
    mWrap = 0l;
    m_isOpen = false;
}
Ticket* ResourceEvolution::requestSaveTicket() {
    if ( !addressBook() ) return 0;
    return createTicket( this );
}
/*
 * skip the first key
 */

bool ResourceEvolution::load() {
    /* doOpen never get's called :( */
    if (!doOpen()) return false;
    if (!mWrap ) return false; // open first!

    DBIterator it = mWrap->begin();
    // skip the "PAS-DB-VERSION"

    for ( ; it != mWrap->end(); ++it ) {
        if ( it.key().startsWith("PAS-DB-VERSION") )
            continue;

        tqWarning( "val:%s", it.value().latin1() );
        VCardTool tool;
        TQString str = it.value().stripWhiteSpace();
        Addressee::List list = tool.parseVCards( str );
        if (!list.first().isEmpty() ) {
            Addressee adr = list.first();
            adr.setResource(this);
            addressBook()->insertAddressee( adr );
        }
    }
    return true;
}
bool ResourceEvolution::save( Ticket* ticket ) {
    delete ticket;
    if (!m_isOpen ) return false;

    // just delete the summary so evolution will regenerate it 
    // on next start up
    (void)TQFile::remove( TQDir::homeDirPath() + "/evolution/local/Contacts/addressbook.db.summary" );


    AddressBook::Iterator it;
    Addressee::List list;
    for ( it = addressBook()->begin(); it !=addressBook()->end(); ++it ) {
        if ( (*it).resource() != this || !(*it).changed() )
            continue;

	// remove, convert add set unchanged false
        list.clear();
        mWrap->remove( (*it).uid() );
        VCardTool tool;
        list.append( (*it) );
        mWrap->add( (*it).uid(), tool.createVCards( list,  VCard::v2_1) );

	(*it).setChanged( false );
    }

    return true;
}
void ResourceEvolution::removeAddressee( const Addressee& rem) {
    if (!m_isOpen) return;

    mWrap->remove( rem.uid() );
}
