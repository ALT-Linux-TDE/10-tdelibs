/*
   This file is part of the KDE libraries
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

#ifndef KICONVIEWSEARCHLINE_H
#define KICONVIEWSEARCHLINE_H

#include <klineedit.h>
#include <tqvaluelist.h>

class TQIconView;
class TQIconViewItem;

/**
 * This class makes it easy to add a search line for filtering the items in a
 * iconview based on a simple text search.
 *
 * No changes to the application other than instantiating this class with an
 * appropriate TQIconView should be needed.
 *
 * @since 3.3
 */
class TDEUI_EXPORT TDEIconViewSearchLine : public KLineEdit
{
  TQ_OBJECT

public:

  /**
   * Constructs a TDEIconViewSearchLine with \a iconView being the TQIconView to
   * be filtered.
   *
   * If \a iconView is null then the widget will be disabled until a iconview
   * is set with setIconView().
   */
  TDEIconViewSearchLine( TQWidget *parent = 0,
		       TQIconView *iconView = 0,
		       const char *name = 0 );

  /**
   * Constructs a TDEIconViewSearchLine without any TQIconView to filter. The
   * TQIconView object has to be set later with setIconView().
   */
  TDEIconViewSearchLine( TQWidget *parent, const char *name );

  /**
   * Destroys the TDEIconViewSearchLine.
   */
  virtual ~TDEIconViewSearchLine();

  /**
   * Returns true if the search is case sensitive.  This defaults to false.
   *
   * @see setCaseSensitive()
   */
  bool caseSensitive() const;

  /**
   * Returns the iconview that is currently filtered by the search.
   *
   * @see setIconView()
   */
  TQIconView *iconView() const;


public slots:
  /**
   * Updates search to only make visible the items that match \a s.  If
   * \a s is null then the line edit's text will be used.
   */
  virtual void updateSearch( const TQString &s = TQString::null );

  /**
   * Make the search case sensitive or case insensitive.
   *
   * @see caseSenstive()
   */
  void setCaseSensitive( bool cs );

  /**
   * Sets the TQIconView that is filtered by this search line.  If \a lv is null
   * then the widget will be disabled.
   *
   * @see iconView()
   */
  void setIconView( TQIconView *iv );


  /**
   * Clear line edit and empty hiddenItems, returning elements to iconView.
   */
  void clear();

  /**
   * Should be called before updateSearch() whenever an icon is deleted.
   */
  void iconDeleted(const TQString &filename);

protected:
  /**
   * Returns true if \a item matches the search \a s.  This will be evaluated
   * based on the value of caseSensitive().  This can be overridden in
   * subclasses to implement more complicated matching schemes.
   */
  virtual bool itemMatches( const TQIconViewItem *item,
			    const TQString &s ) const;


  /**
   * Do initialization common to both constructors.
   */
  void init( TQIconView *iconView = 0 );

  /**
   * Hide item.
   */
  void hideItem( TQIconViewItem *item );

  /**
   * Show item.
   *
   * Just unhide it, doesn't necessary show it on screen, for that use
   * iconView->ensureItemVisible()
   */
  void showItem( TQIconViewItem *item );


protected slots:
  /**
   * When keys are pressed a new search string is created and a timer is
   * activated.  The most recent search is activated when this timer runs out
   * if another key has not yet been pressed.
   *
   * This method makes @param s the most recent search and starts the
   * timer.
   *
   * Together with activateSearch() this makes it such that searches are not
   * started until there is a short break in the users typing.
   *
   * @see activateSearch()
   */
  void queueSearch( const TQString &s );

  /**
   * When the timer started with queueSearch() expires this slot is called.
   * If there has been another timer started then this slot does nothing.
   * However if there are no other pending searches this starts the icon view
   * search.
   *
   * @see queueSearch()
   */
  void activateSearch();


private slots:
  /**
   * Take action need when iconView is deleted.
   */
  void iconViewDeleted();


private:
  class TDEIconViewSearchLinePrivate;
  TDEIconViewSearchLinePrivate *d;
};


#endif /* KICONVIEWSEARCHLINE_H */
