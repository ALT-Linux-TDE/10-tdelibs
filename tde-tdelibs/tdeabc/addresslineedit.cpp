/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
                  2002 Lubos Lunak <llunak@suse.cz>
                  2001,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// $Id$

#include "addresslineedit.h"

#include <tqapplication.h>
#include <tqobject.h>
#include <tqptrlist.h>
#include <tqregexp.h>
#include <tqevent.h>
#include <tqdragobject.h>

#include <tdecompletionbox.h>
#include <tdeconfig.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <tdestdaccel.h>
#include <kurldrag.h>

#include <tdeabc/stdaddressbook.h>
#include <tdeabc/distributionlist.h>
#include "ldapclient.h"

#include <kdebug.h>

//=============================================================================
//
//   Class  AddressLineEdit
//
//=============================================================================


using namespace TDEABC;

TDECompletion * AddressLineEdit::s_completion = 0L;
bool AddressLineEdit::s_addressesDirty = false;
TQTimer* AddressLineEdit::s_LDAPTimer = 0L;
LdapSearch* AddressLineEdit::s_LDAPSearch = 0L;
TQString* AddressLineEdit::s_LDAPText = 0L;
AddressLineEdit* AddressLineEdit::s_LDAPLineEdit = 0L;
TDEConfig *AddressLineEdit::s_config = 0L;

static KStaticDeleter<TDECompletion> completionDeleter;
static KStaticDeleter<TQTimer> ldapTimerDeleter;
static KStaticDeleter<LdapSearch> ldapSearchDeleter;
static KStaticDeleter<TQString> ldapTextDeleter;
static KStaticDeleter<TDEConfig> configDeleter;

AddressLineEdit::AddressLineEdit(TQWidget* parent,
		bool useCompletion,
		const char *name)
    : KLineEdit(parent,name)
{
  m_useCompletion = useCompletion;
  m_completionInitialized = false;
  m_smartPaste = false;

  init();

  // Whenever a new AddressLineEdit is created (== a new composer is created),
  // we set a dirty flag to reload the addresses upon the first completion.
  // The address completions are shared between all AddressLineEdits.
  // Is there a signal that tells us about addressbook updates?
  if (m_useCompletion)
    s_addressesDirty = true;
}


//-----------------------------------------------------------------------------
void AddressLineEdit::init()
{
  if ( !s_completion ) {
      completionDeleter.setObject( s_completion, new TDECompletion() );
      s_completion->setOrder( TDECompletion::Sorted );
      s_completion->setIgnoreCase( true );
  }

  if( m_useCompletion ) {
      if( !s_LDAPTimer ) {
        ldapTimerDeleter.setObject( s_LDAPTimer, new TQTimer );
        ldapSearchDeleter.setObject( s_LDAPSearch, new LdapSearch );
        ldapTextDeleter.setObject( s_LDAPText, new TQString );
      }
      connect( s_LDAPTimer, TQ_SIGNAL( timeout()), TQ_SLOT( slotStartLDAPLookup()));
      connect( s_LDAPSearch, TQ_SIGNAL( searchData( const TQStringList& )),
        TQ_SLOT( slotLDAPSearchData( const TQStringList& )));
  }

  if ( m_useCompletion && !m_completionInitialized )
  {
      setCompletionObject( s_completion, false ); // we handle it ourself
      connect( this, TQ_SIGNAL( completion(const TQString&)),
               this, TQ_SLOT(slotCompletion() ));
      
      TDECompletionBox *box = completionBox();
      connect( box, TQ_SIGNAL( highlighted( const TQString& )),
               this, TQ_SLOT( slotPopupCompletion( const TQString& ) ));
      connect( box, TQ_SIGNAL( userCancelled( const TQString& )),
               TQ_SLOT( userCancelled( const TQString& )));

      m_completionInitialized = true; // don't connect muliple times. That's
                                      // ugly, tho, better have completionBox()
                                      // virtual in KDE 4
      // Why? This is only called once. Why should this be called more
      // than once? And why was this protected?
  }
}

//-----------------------------------------------------------------------------
AddressLineEdit::~AddressLineEdit()
{
}

//-----------------------------------------------------------------------------

TDEConfig* AddressLineEdit::config()
{
  if ( !s_config )
    configDeleter.setObject( s_config, new TDEConfig( "kabldaprc", false, false ) ); // Open read-write, no kdeglobals

  return s_config;
}

void AddressLineEdit::setFont( const TQFont& font )
{
    KLineEdit::setFont( font );
    if ( m_useCompletion )
        completionBox()->setFont( font );
}

//-----------------------------------------------------------------------------
void AddressLineEdit::keyPressEvent(TQKeyEvent *e)
{
    bool accept = false;

    if (TDEStdAccel::shortcut(TDEStdAccel::SubstringCompletion).contains(KKey(e)))
    {
        doCompletion(true);
        accept = true;
    }
    else if (TDEStdAccel::shortcut(TDEStdAccel::TextCompletion).contains(KKey(e)))
    {
        int len = text().length();
        
        if (len == cursorPosition()) // at End?
        {
            doCompletion(true);
            accept = true;
        }
    }

    if( !accept )
        KLineEdit::keyPressEvent( e );

    if( e->isAccepted())
    {
        if( m_useCompletion && s_LDAPTimer != NULL )
        {
            if( *s_LDAPText != text())
                stopLDAPLookup();
            *s_LDAPText = text();
            s_LDAPLineEdit = this;
            s_LDAPTimer->start( 500, true );
        }
    }
}

void AddressLineEdit::mouseReleaseEvent( TQMouseEvent * e )
{
   if (m_useCompletion && (e->button() == TQt::MidButton))
   {
      m_smartPaste = true;
      KLineEdit::mouseReleaseEvent(e);
      m_smartPaste = false;
      return;
   }
   KLineEdit::mouseReleaseEvent(e);
}

void AddressLineEdit::insert(const TQString &t)
{
    if (!m_smartPaste)
    {
       KLineEdit::insert(t);
       return;
    }
    TQString newText = t.stripWhiteSpace();
    if (newText.isEmpty())
       return;

    // remove newlines in the to-be-pasted string as well as an eventual
    // mailto: protocol
    newText.replace( TQRegExp("\r?\n"), ", " );
    if ( newText.startsWith( "mailto:" ) )
    {
      KURL u(newText);
      newText = u.path();
    }
    else if (newText.find(" at ") != -1)
    {
       // Anti-spam stuff
       newText.replace( " at ", "@" );
       newText.replace( " dot ", "." );
    }
    else if (newText.find("(at)") != -1)
    {
      newText.replace( TQRegExp("\\s*\\(at\\)\\s*"), "@" );
    }

    TQString contents = text();
    int start_sel = 0;
    int end_sel = 0;
    int pos = cursorPosition();
    if (getSelection(&start_sel, &end_sel))
    {
       // Cut away the selection.
       if (pos > end_sel)
          pos -= (end_sel - start_sel);
       else if (pos > start_sel)
          pos = start_sel;
       contents = contents.left(start_sel) + contents.right(end_sel+1);
    }

    int eot = contents.length();
    while ((eot > 0) && contents[eot-1].isSpace()) eot--;
    if (eot == 0)
    {
       contents = TQString::null;
    }
    else if (pos >= eot)
    {
       if (contents[eot-1] == ',')
          eot--;
       contents.truncate(eot);
       contents += ", ";
       pos = eot+2;
    }

    contents = contents.left(pos)+newText+contents.mid(pos);
    setText(contents);
    setCursorPosition(pos+newText.length());
}

void AddressLineEdit::paste()
{
    if (m_useCompletion)
       m_smartPaste = true;
    KLineEdit::paste();
    m_smartPaste = false;
}

//-----------------------------------------------------------------------------
void AddressLineEdit::cursorAtEnd()
{
    setCursorPosition( text().length() );
}

//-----------------------------------------------------------------------------
void AddressLineEdit::enableCompletion(bool enable)
{
  m_useCompletion = enable;
}

//-----------------------------------------------------------------------------
void AddressLineEdit::doCompletion(bool ctrlT)
{
    if ( !m_useCompletion )
        return;

    TQString prevAddr;    
    
    TQString s(text());    
    int n = s.findRev(',');
        
    if (n >= 0)
    {
        n++; // Go past the ","
        
        int len = s.length();
        
        // Increment past any whitespace...
        while( n < len && s[n].isSpace() )
          n++;
                  
        prevAddr = s.left(n);
        s = s.mid(n,255).stripWhiteSpace();
    }

    if ( s_addressesDirty )
        loadAddresses();

    if ( ctrlT )
    {
        TQStringList completions = s_completion->substringCompletion( s );
        if (completions.count() > 1) {
            m_previousAddresses = prevAddr;
            setCompletedItems( completions );
        }
        else if (completions.count() == 1)
            setText(prevAddr + completions.first());

        cursorAtEnd();
        return;
    }
    
    TDEGlobalSettings::Completion  mode = completionMode();
    
    switch ( mode )
    {
        case TDEGlobalSettings::CompletionPopupAuto:
        {
            if (s.isEmpty())
                break;
        }
        case TDEGlobalSettings::CompletionPopup:        
        {
            m_previousAddresses = prevAddr;
            TQStringList items = s_completion->allMatches( s );
            items += s_completion->allMatches( "\"" + s );
            items += s_completion->substringCompletion( '<' + s );
            uint beforeDollarCompletionCount = items.count();

            if( s.find( ' ' ) == -1 ) // one word, possibly given name
                items += s_completion->allMatches( "$$" + s );

            if ( !items.isEmpty() )
            {
                if ( items.count() > beforeDollarCompletionCount )
                {
                    // remove the '$$whatever$' part                    
                    for( TQStringList::Iterator it = items.begin();
                         it != items.end();
                         ++it )
                    { 
                        int pos = (*it).find( '$', 2 );
                        if( pos < 0 ) // ???
                            continue;
                        (*it)=(*it).mid( pos + 1 );
                    }
                }

                items = removeMailDupes( items );
                
                // We do not want KLineEdit::setCompletedItems to perform text
                // completion (suggestion) since it does not know how to deal
                // with providing proper completions for different items on the 
                // same line, e.g. comma-separated list of email addresses.
                bool autoSuggest = (mode != TDEGlobalSettings::CompletionPopupAuto);                                
                setCompletedItems( items, autoSuggest );
                
                if (!autoSuggest)
                {
                    int index = items.first().find( s );
                    TQString newText = prevAddr + items.first().mid( index );
                    //kdDebug() << "OLD TEXT: " << text() << endl;
                    //kdDebug() << "NEW TEXT: " << newText << endl;
                    setUserSelection(false);
                    setCompletedText(newText,true);
                }
            }

            break;
        }

        case TDEGlobalSettings::CompletionShell:
        {
            TQString match = s_completion->makeCompletion( s );
            if ( !match.isNull() && match != s )
            {
                setText( prevAddr + match );
                cursorAtEnd();
            }
            break;
        }

        case TDEGlobalSettings::CompletionMan: // Short-Auto in fact
        case TDEGlobalSettings::CompletionAuto:
        {
            if (!s.isEmpty())
            {              
                TQString match = s_completion->makeCompletion( s );
                if ( !match.isNull() && match != s )
                {
                  TQString adds = prevAddr + match;
                  setCompletedText( adds );
                }
                break;
            }
        }        
        case TDEGlobalSettings::CompletionNone:
        default: // fall through        
            break;
    }
}

//-----------------------------------------------------------------------------
void AddressLineEdit::slotPopupCompletion( const TQString& completion )
{
    setText( m_previousAddresses + completion );
    cursorAtEnd();
}

//-----------------------------------------------------------------------------
void AddressLineEdit::loadAddresses()
{
    s_completion->clear();
    s_addressesDirty = false;

    TQStringList adrs = addresses();
    for( TQStringList::ConstIterator it = adrs.begin(); it != adrs.end(); ++it)
        addAddress( *it );
}

void AddressLineEdit::addAddress( const TQString& adr )
{
    s_completion->addItem( adr );
    int pos = adr.find( '<' );
    if( pos >= 0 )
    {
        ++pos;
        int pos2 = adr.find( pos, '>' );
        if( pos2 >= 0 )
            s_completion->addItem( adr.mid( pos, pos2 - pos ));
    }
}

void AddressLineEdit::slotStartLDAPLookup()
{
    if( !s_LDAPSearch->isAvailable() || s_LDAPLineEdit != this )
        return;
    startLoadingLDAPEntries();
}

void AddressLineEdit::stopLDAPLookup()
{
    s_LDAPSearch->cancelSearch();
    s_LDAPLineEdit = NULL;
}

void AddressLineEdit::startLoadingLDAPEntries()
{
    TQString s( *s_LDAPText );
    // TODO cache last?
    TQString prevAddr;
    int n = s.findRev(',');
    if (n>= 0)
    {
        prevAddr = s.left(n+1) + ' ';
        s = s.mid(n+1,255).stripWhiteSpace();
    }
    if( s.length() == 0 )
        return;
    
    loadAddresses(); // TODO reuse these?
    s_LDAPSearch->startSearch( s );
}

void AddressLineEdit::slotLDAPSearchData( const TQStringList& adrs )
{
    if( s_LDAPLineEdit != this )
        return;
    for( TQStringList::ConstIterator it = adrs.begin(); it != adrs.end(); ++it ) {
        TQString name(*it);
        int pos = name.find( " <" );
        int pos_comma = name.find( ',' );
        // put name in quotes, if we have a comma in the name
        if (pos>0 && pos_comma>0 && pos_comma<pos) {
          name.insert(pos, '\"');
          name.prepend('\"');
        }
        addAddress( name );
    }
    
    if( hasFocus() || completionBox()->hasFocus())
    {
        if( completionMode() != TDEGlobalSettings::CompletionNone )
        {
            doCompletion( false );
        }
    }
}

TQStringList AddressLineEdit::removeMailDupes( const TQStringList& adrs )
{
    TQStringList src = adrs;
    qHeapSort( src );
    TQString last;
    for( TQStringList::Iterator it = src.begin(); it != src.end(); ) {
        if( *it == last )
        {
            it = src.remove( it );
            continue; // dupe
        }
        last = *it;
        ++it;
    }
    return src;
}

//-----------------------------------------------------------------------------
void AddressLineEdit::dropEvent(TQDropEvent *e)
{
  KURL::List uriList;
  if(KURLDrag::canDecode(e) && KURLDrag::decode( e, uriList ))
  {
    TQString ct = text();
    KURL::List::Iterator it = uriList.begin();
    for (; it != uriList.end(); ++it)
    {
      if (!ct.isEmpty()) ct.append(", ");
      KURL u(*it);
      if ((*it).protocol() == "mailto")
          ct.append( (*it).path() );
      else
          ct.append( (*it).url() );
    }
    setText(ct);
    setEdited( true );
  }
  else {
    if (m_useCompletion)
       m_smartPaste = true;
    TQLineEdit::dropEvent(e);
    m_smartPaste = false;
  }
}


TQStringList AddressLineEdit::addresses()
{
  TQApplication::setOverrideCursor( KCursor::waitCursor() ); // loading might take a while

  TQStringList result;
  TQString space(" ");
  TQRegExp needQuotes("[^ 0-9A-Za-z\\x0080-\\xFFFF]");
  TQString endQuote("\" ");
  TQString addr, email;

  TDEABC::AddressBook *addressBook = TDEABC::StdAddressBook::self();
  TDEABC::AddressBook::Iterator it;
  for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    TQStringList emails = (*it).emails();
    
    TQString n = (*it).prefix() + space +
                (*it).givenName() + space +
                (*it).additionalName() + space +
                (*it).familyName() + space +
                (*it).suffix();
    
    n = n.simplifyWhiteSpace();

    TQStringList::ConstIterator mit;

    for ( mit = emails.begin(); mit != emails.end(); ++mit ) {
      email = *mit;
      if (!email.isEmpty()) {
        if (n.isEmpty() || (email.find( '<' ) != -1))
          addr = TQString::null;
        else { /* do we really need quotes around this name ? */
                if (n.find(needQuotes) != -1)
            addr = '"' + n + endQuote;
          else
            addr = n + space;
        }

        if (!addr.isEmpty() && (email.find( '<' ) == -1)
            && (email.find( '>' ) == -1)
            && (email.find( ',' ) == -1))
          addr += '<' + email + '>';
        else
          addr += email;
        addr = addr.stripWhiteSpace();
        result.append( addr );
      }
    }
  }
  
  TDEABC::DistributionListManager manager( addressBook );
  manager.load();
  result += manager.listNames();

  TQApplication::restoreOverrideCursor();

  return result;
}

#include "addresslineedit.moc"
