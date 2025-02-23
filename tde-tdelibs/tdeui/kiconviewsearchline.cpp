/* This file is part of the KDE libraries
   Copyright (c) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>
   Copyright (c) 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
 * \todo
 *     Maybe we should have a common interface for SearchLines, this file
 * is so close (it's actually based on) tdelistviewsearchline! Only few methods
 * would be reimplemented.
 */

#include "kiconviewsearchline.h"

#include <tqiconview.h>
#include <tdelocale.h>
#include <tqtimer.h>
#include <kdebug.h>

#define DEFAULT_CASESENSITIVE false

typedef TQValueList <TQIconViewItem *> QIconViewItemList;

class TDEIconViewSearchLine::TDEIconViewSearchLinePrivate
{
public:
  TDEIconViewSearchLinePrivate() :
    iconView( 0 ),
    caseSensitive( DEFAULT_CASESENSITIVE ),
    activeSearch( false ),
    queuedSearches( 0 ) {}

  TQIconView *iconView;
  bool caseSensitive;
  bool activeSearch;
  TQString search;
  int queuedSearches;
};

/******************************************************************************
 * Public Methods                                                             *
 *****************************************************************************/
TDEIconViewSearchLine::TDEIconViewSearchLine( TQWidget *parent,
					  TQIconView *iconView,
					  const char *name ) :
  KLineEdit( parent, name )
{
  d = NULL;
  init( iconView );
}

TDEIconViewSearchLine::TDEIconViewSearchLine( TQWidget *parent, const char *name ) :
  KLineEdit( parent, name )
{
  d = NULL;
  init( NULL );
}

TDEIconViewSearchLine::~TDEIconViewSearchLine()
{
  clear(); // empty hiddenItems, returning items back to iconView
  delete d;
}

bool TDEIconViewSearchLine::caseSensitive() const
{
  return d->caseSensitive;
}

TQIconView *TDEIconViewSearchLine::iconView() const
{
  return d->iconView;
}

/******************************************************************************
 * Public Slots                                                               *
 *****************************************************************************/
void TDEIconViewSearchLine::updateSearch( const TQString &s )
{
	if( ! d->iconView )
		return;

	d->search = s.isNull() ? text() : s;
	TQIconViewItem *currentItem = d->iconView->currentItem();
	TQIconViewItem *item = NULL;

	// Remove Non-Matching items, add them to the hidden list
	TQIconViewItem *i = d->iconView->firstItem();
	while ( i != NULL ) {
		item = i;
		i = i->nextItem(); // Point to next, otherwise will loose it.
		if ( ! itemMatches( item, d->search ) ) {
			hideItem( item );

			if ( item == currentItem )
				currentItem = NULL; // It's not in iconView anymore.
		}
		else {
			showItem( item );
		}
	}

	d->iconView->sort(); // This also arranges items in grid

	if ( currentItem != NULL )
		d->iconView->ensureItemVisible( currentItem );
}

void TDEIconViewSearchLine::clear()
{
	if( ! d->iconView )
		return; // disabled

	// Clear hidden list, give items back to TQIconView, if it still exists
	TQIconViewItem *item = NULL;

	TQIconViewItem *i = d->iconView->firstItem();
	while ( i != NULL ) {
		item = i;
		i = i->nextItem(); // Point to next, otherwise will loose it.
		showItem( item );
	}
	
	d->search = "";
	d->queuedSearches = 0;
	KLineEdit::clear();
}

void TDEIconViewSearchLine::iconDeleted(const TQString &filename) {
	// Do nothing...
}

void TDEIconViewSearchLine::setCaseSensitive( bool cs )
{
  d->caseSensitive = cs;
}

void TDEIconViewSearchLine::setIconView( TQIconView *iv )
{
  if ( d->iconView != NULL )
    disconnect( d->iconView, TQ_SIGNAL( destroyed() ),
		this,        TQ_SLOT(   iconViewDeleted() ) );

  d->iconView = iv;

  if ( iv != NULL )
    {
      connect( d->iconView, TQ_SIGNAL( destroyed() ),
	       this,        TQ_SLOT(   iconViewDeleted() ) );
      setEnabled( true );
    }
  else
    setEnabled( false );
}

/******************************************************************************
 * Protected Methods                                                          *
 *****************************************************************************/
bool TDEIconViewSearchLine::itemMatches( const TQIconViewItem *item,
				       const TQString &s ) const
{
  if ( s.isEmpty() )
    return true;

  if ( item == NULL )
    return false;

  TQString itemtext = item->text();
  return ( itemtext.find( s, 0, caseSensitive() ) >= 0 );
}

void TDEIconViewSearchLine::init( TQIconView *iconView )
{
  delete d;
  d = new TDEIconViewSearchLinePrivate;

  d->iconView = iconView;

  connect( this, TQ_SIGNAL( textChanged( const TQString & ) ),
	   this, TQ_SLOT(   queueSearch( const TQString & ) ) );

  if ( iconView != NULL )
    {
      connect( iconView, TQ_SIGNAL( destroyed() ),
	       this,     TQ_SLOT(   iconViewDeleted() ) );
      setEnabled( true );
    }
  else
    setEnabled( false );
}

void TDEIconViewSearchLine::hideItem( TQIconViewItem *item )
{
  if ( ( item == NULL ) || ( d->iconView == NULL ) )
    return;

  item->setVisible(false);
}

void TDEIconViewSearchLine::showItem( TQIconViewItem *item )
{
  if ( d->iconView == NULL )
    {
      kdDebug() << __FILE__ << ":" << __LINE__ <<
	"showItem() could not be called while there's no iconView set." <<
	endl;
      return;
    }

  item->setVisible(true);
}

/******************************************************************************
 * Protected Slots                                                            *
 *****************************************************************************/
void TDEIconViewSearchLine::queueSearch( const TQString &s )
{
  d->queuedSearches++;
  d->search = s;
  TQTimer::singleShot( 200, this, TQ_SLOT( activateSearch() ) );
}

void TDEIconViewSearchLine::activateSearch()
{
  d->queuedSearches--;

  if ( d->queuedSearches <= 0 )
  {
      updateSearch( d->search );
      d->queuedSearches = 0;
  }
  else {
      TQTimer::singleShot( 200, this, TQ_SLOT( activateSearch() ) );
  }
}

/******************************************************************************
 * Private Slots                                                              *
 *****************************************************************************/
void TDEIconViewSearchLine::iconViewDeleted()
{
  d->iconView = NULL;
  setEnabled( false );
}

#include "kiconviewsearchline.moc"
