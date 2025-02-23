#include <tqstring.h>
#include <tqvariant.h>

#include <tdeaccel.h>
#include "tdeaccelaction.h"
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kglobalaccel.h>
#include <kinstance.h>
#include <tdeshortcut.h>
#include "tdeshortcutlist.h"

//---------------------------------------------------------------------
// TDEShortcutList
//---------------------------------------------------------------------

TDEShortcutList::TDEShortcutList()
{
}

TDEShortcutList::~TDEShortcutList()
{
}

bool TDEShortcutList::isGlobal( uint ) const
{
	return false;
}

int TDEShortcutList::index( const TQString& sName ) const
{
	uint nSize = count();
        for( uint i = 0;
             i < nSize;
             ++i )
            if( name( i ) == sName )
                return i;
	return -1;
}

int TDEShortcutList::index( const KKeySequence& seq ) const
{
	if( seq.isNull() )
		return -1;

	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		if( shortcut(i).contains( seq ) )
			return i;
	}

	return -1;
}

const TDEInstance* TDEShortcutList::instance() const
{
	return 0;
}

TQVariant TDEShortcutList::getOther( Other, uint ) const
{
	return TQVariant();
}

bool TDEShortcutList::setOther( Other, uint, TQVariant )
{
	return false;
}

bool TDEShortcutList::readSettings( const TQString& sConfigGroup, TDEConfigBase* pConfig )
{
	kdDebug(125) << "TDEShortcutList::readSettings( \"" << sConfigGroup << "\", " << pConfig << " ) start" << endl;
	if( !pConfig )
		pConfig = TDEGlobal::config();
	TQString sGroup = (!sConfigGroup.isEmpty()) ? sConfigGroup : TQString("Shortcuts");

	// If the config file still has the old group name:
	// FIXME: need to rename instead? -- and don't do this if hasGroup( "Shortcuts" ).
	if( sGroup == "Shortcuts" && pConfig->hasGroup( "Keys" ) ) {
		readSettings( "Keys", pConfig );
	}

	kdDebug(125) << "\treadSettings( \"" << sGroup << "\", " << pConfig << " )" << endl;
	if( !pConfig->hasGroup( sGroup ) )
		return true;
	TDEConfigGroupSaver cgs( pConfig, sGroup );

	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		if( isConfigurable(i) ) {
			TQString sEntry = pConfig->readEntry( name(i) );
			if( !sEntry.isEmpty() ) {
				if( sEntry == "none" )
					setShortcut( i, TDEShortcut() );
				else
					setShortcut( i, TDEShortcut(sEntry) );
			}
			else // default shortcut
				setShortcut( i, shortcutDefault(i) );
			kdDebug(125) << "\t" << name(i) << " = '" << sEntry << "'" << endl;
		}
	}

	kdDebug(125) << "TDEShortcutList::readSettings done" << endl;
	return true;
}

bool TDEShortcutList::writeSettings( const TQString &sConfigGroup, TDEConfigBase* pConfig, bool bWriteAll, bool bGlobal ) const
{
	kdDebug(125) << "TDEShortcutList::writeSettings( " << sConfigGroup << ", " << pConfig << ", " << bWriteAll << ", " << bGlobal << " )" << endl;
	if( !pConfig )
		pConfig = TDEGlobal::config();

	TQString sGroup = (!sConfigGroup.isEmpty()) ? sConfigGroup : TQString("Shortcuts");

	// If it has the deprecated group [Keys], remove it
	if( pConfig->hasGroup( "Keys" ) )
		pConfig->deleteGroup( "Keys", true );

	TDEConfigGroupSaver cs( pConfig, sGroup );

	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		if( isConfigurable(i) ) {
			const TQString& sName = name(i);
			bool bConfigHasAction = !pConfig->readEntry( sName ).isEmpty();
			bool bSameAsDefault = (shortcut(i) == shortcutDefault(i));
			// If we're using a global config or this setting
			//  differs from the default, then we want to write.
			if( bWriteAll || !bSameAsDefault ) {
				TQString s = shortcut(i).toStringInternal();
				if( s.isEmpty() )
					s = "none";
				kdDebug(125) << "\twriting " << sName << " = " << s << endl;
				pConfig->writeEntry( sName, s, true, bGlobal );
			}
			// Otherwise, this key is the same as default
			//  but exists in config file.  Remove it.
			else if( bConfigHasAction ) {
				kdDebug(125) << "\tremoving " << sName << " because == default" << endl;
				pConfig->deleteEntry( sName, false, bGlobal );
			}
		}
	}

	pConfig->sync();
	return true;
}

//---------------------------------------------------------------------
// TDEAccelShortcutList
//---------------------------------------------------------------------

class TDEAccelShortcutListPrivate
{
	public:
		TQString m_configGroup;
};

TDEAccelShortcutList::TDEAccelShortcutList( TDEAccel* pAccel )
: m_actions( pAccel->actions() )
{
	d=new TDEAccelShortcutListPrivate;
	m_bGlobal = false;
	d->m_configGroup=pAccel->configGroup();
}

TDEAccelShortcutList::TDEAccelShortcutList( TDEGlobalAccel* pAccel )
: m_actions( pAccel->actions() )
{
	d=new TDEAccelShortcutListPrivate;
	m_bGlobal = true;
	d->m_configGroup=pAccel->configGroup();
}

TDEAccelShortcutList::TDEAccelShortcutList( TDEAccelActions& actions, bool bGlobal )
: m_actions( actions )
{
	d=new TDEAccelShortcutListPrivate;
	m_bGlobal = bGlobal;
}


TDEAccelShortcutList::~TDEAccelShortcutList()
	{  delete d;}
uint TDEAccelShortcutList::count() const
	{ return m_actions.count(); }
TQString TDEAccelShortcutList::name( uint i ) const
	{ return m_actions.actionPtr(i)->name(); }
TQString TDEAccelShortcutList::label( uint i ) const
	{ return m_actions.actionPtr(i)->label(); }
TQString TDEAccelShortcutList::whatsThis( uint i ) const
	{ return m_actions.actionPtr(i)->whatsThis(); }
const TDEShortcut& TDEAccelShortcutList::shortcut( uint i ) const
	{ return m_actions.actionPtr(i)->shortcut(); }
const TDEShortcut& TDEAccelShortcutList::shortcutDefault( uint i ) const
	{ return m_actions.actionPtr(i)->shortcutDefault(); }
bool TDEAccelShortcutList::isConfigurable( uint i ) const
	{ return m_actions.actionPtr(i)->isConfigurable(); }
bool TDEAccelShortcutList::setShortcut( uint i, const TDEShortcut& cut )
	{ return m_actions.actionPtr(i)->setShortcut( cut ); }
TQVariant TDEAccelShortcutList::getOther( Other, uint ) const
	{ return TQVariant(); }
bool TDEAccelShortcutList::isGlobal( uint ) const
	{ return m_bGlobal; }
bool TDEAccelShortcutList::setOther( Other, uint, TQVariant )
	{ return false; }
bool TDEAccelShortcutList::save() const
	{ return writeSettings( d->m_configGroup ); }

void TDEShortcutList::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void TDEAccelShortcutList::virtual_hook( int id, void* data )
{ TDEShortcutList::virtual_hook( id, data ); }

void TDEStdAccel::ShortcutList::virtual_hook( int id, void* data )
{ TDEShortcutList::virtual_hook( id, data ); }

