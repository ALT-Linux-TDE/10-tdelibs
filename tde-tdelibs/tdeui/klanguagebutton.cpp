/*
 * klanguagebutton.cpp - Adds some methods for inserting languages.
 *
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define INCLUDE_MENUITEM_DEF
#include <tqpopupmenu.h>
#include <tqlayout.h>
#include <tqpushbutton.h>

#include "klanguagebutton.h"
#include "klanguagebutton.moc"

#include <kdebug.h>

static void checkInsertPos( TQPopupMenu *popup, const TQString & str,
                            int &index )
{
  if ( index == -1 )
    return;

  int a = 0;
  int b = popup->count();
  while ( a < b )
  {
    int w = ( a + b ) / 2;

    int id = popup->idAt( w );
    int j = str.localeAwareCompare( popup->text( id ) );

    if ( j > 0 )
      a = w + 1;
    else
      b = w;
  }

  index = a; // it doesn't really matter ... a == b here.

  Q_ASSERT( a == b );
}

static TQPopupMenu * checkInsertIndex( TQPopupMenu *popup,
                                      const TQStringList *tags, const TQString &submenu )
{
  int pos = tags->findIndex( submenu );

  TQPopupMenu *pi = 0;
  if ( pos != -1 )
  {
    TQMenuItem *p = popup->findItem( pos );
    pi = p ? p->popup() : 0;
  }
  if ( !pi )
    pi = popup;

  return pi;
}

class KLanguageButtonPrivate
{
public:
  TQPushButton * button;
  bool staticText;
};

KLanguageButton::KLanguageButton( TQWidget * parent, const char *name )
  : TQWidget( parent, name )
{
  init(name);
}

KLanguageButton::KLanguageButton( const TQString & text, TQWidget * parent, const char *name )
  : TQWidget( parent, name )
{
  init(name);

  setText(text);
}

void KLanguageButton::setText(const TQString & text)
{
  d->staticText = true;
  d->button->setText(text);
  d->button->setIconSet(TQIconSet()); // remove the icon
}

void KLanguageButton::init(const char * name)
{
  m_current = 0;
  m_ids = new TQStringList;
  m_popup = 0;
  m_oldPopup = 0;
  d = new KLanguageButtonPrivate;

  d->staticText = false;

  TQHBoxLayout *layout = new TQHBoxLayout(this, 0, 0);
  layout->setAutoAdd(true);
  d->button = new TQPushButton( this, name ); // HPB don't touch this!!

  clear();
}

KLanguageButton::~KLanguageButton()
{
  delete m_ids;

  delete d->button;
  delete d;
}


void KLanguageButton::insertLanguage( const TQString& path, const TQString& name,
                        const TQString&, const TQString &submenu, int index )
{
  TQString output = name + TQString::fromLatin1( " (" ) + path +
                   TQString::fromLatin1( ")" );
#if 0
  // Nooooo ! Country != language
  TQPixmap flag( locate( "locale", sub + path +
                TQString::fromLatin1( "/flag.png" ) ) );
#endif
  insertItem( output, path, submenu, index );
}

void KLanguageButton::insertItem( const TQIconSet& icon, const TQString &text,
                                  const TQString & id, const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, count(), index );
  m_ids->append( id );
}

void KLanguageButton::insertItem( const TQString &text, const TQString & id,
                                  const TQString &submenu, int index )
{
  insertItem( TQIconSet(), text, id, submenu, index );
}

void KLanguageButton::insertSeparator( const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  pi->insertSeparator( index );
  m_ids->append( TQString::null );
}

void KLanguageButton::insertSubmenu( const TQIconSet & icon,
                                     const TQString &text, const TQString &id,
                                     const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_ids, submenu );
  TQPopupMenu *p = new TQPopupMenu( pi );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, p, count(), index );
  m_ids->append( id );
  connect( p, TQ_SIGNAL( activated( int ) ),
           TQ_SLOT( slotActivated( int ) ) );
  connect( p, TQ_SIGNAL( highlighted( int ) ), this,
           TQ_SLOT( slotHighlighted( int ) ) );
}

void KLanguageButton::insertSubmenu( const TQString &text, const TQString &id,
                                     const TQString &submenu, int index )
{
  insertSubmenu(TQIconSet(), text, id, submenu, index);
}

void KLanguageButton::slotActivated( int index )
{
  //kdDebug() << "slotActivated" << index << endl;

  setCurrentItem( index );

  // Forward event from popup menu as if it was emitted from this widget:
  TQString id = *m_ids->at( index );
  emit activated( id );
}

void KLanguageButton::slotHighlighted( int index )
{
  //kdDebug() << "slotHighlighted" << index << endl;

  TQString id = *m_ids->at( index );
  emit ( highlighted(id) );
}

int KLanguageButton::count() const
{
  return m_ids->count();
}

void KLanguageButton::clear()
{
  m_ids->clear();

  delete m_oldPopup;
  m_oldPopup = m_popup;
  m_popup = new TQPopupMenu( this );

  d->button->setPopup( m_popup );

  connect( m_popup, TQ_SIGNAL( activated( int ) ),
           TQ_SLOT( slotActivated( int ) ) );
  connect( m_popup, TQ_SIGNAL( highlighted( int ) ),
           TQ_SLOT( slotHighlighted( int ) ) );

  if ( !d->staticText )
  {
    d->button->setText( TQString::null );
    d->button->setIconSet( TQIconSet() );
  }
}

bool KLanguageButton::contains( const TQString & id ) const
{
  return m_ids->contains( id ) > 0;
}

TQString KLanguageButton::current() const
{
  return *m_ids->at( currentItem() );
}


TQString KLanguageButton::id( int i ) const
{
  if ( i < 0 || i >= count() )
  {
    kdDebug() << "KLanguageButton::tag(), unknown tag " << i << endl;
    return TQString::null;
  }
  return *m_ids->at( i );
}


int KLanguageButton::currentItem() const
{
  return m_current;
}

void KLanguageButton::setCurrentItem( int i )
{
  if ( i < 0 || i >= count() )
    return;
  m_current = i;

  if ( !d->staticText )
  {
    d->button->setText( m_popup->text( m_current ) );
    TQIconSet *icon = m_popup->iconSet( m_current );
    if ( icon )
      d->button->setIconSet( *icon );
    else
      d->button->setIconSet( TQIconSet() );
  }
}

void KLanguageButton::setCurrentItem( const TQString & id )
{
  int i = m_ids->findIndex( id );
  if ( id.isNull() )
    i = 0;
  if ( i != -1 )
    setCurrentItem( i );
}
