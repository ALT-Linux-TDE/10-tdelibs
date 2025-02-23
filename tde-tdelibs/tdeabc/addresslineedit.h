/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
                  2002 Lubos Lunak <llunak@suse.cz>

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

#ifndef KABC_ADDRESSLINEEDIT_H
#define KABC_ADDRESSLINEEDIT_H
// $Id$

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqtimer.h>

#include "klineedit.h"
#include "kcompletion.h"

class TDEConfig;

namespace TDEABC {

class LdapSearch;

/**
 * A lineedit with LDAP and tdeabc completion
 *
 * This lineedit is supposed to be used wherever the user types email addresses
 * and might want a completion. You can simply use it as a replacement for
 * KLineEdit or TQLineEdit.
 *
 * You can enable or disable the lineedit at any time.
 *
 * @see AddressLineEdit::enableCompletion()
 */
class KABC_EXPORT AddressLineEdit : public KLineEdit
{
  TQ_OBJECT
public:
  AddressLineEdit(TQWidget* parent, bool useCompletion = true,
		const char *name = 0L);
  virtual ~AddressLineEdit();

  /**
   * Reimplented for internal reasons.
   * @ see KLineEdit::setFont()
   */
  virtual void setFont( const TQFont& );

  static TDEConfig *config();

public slots:
  /**
   * Set cursor to end of line.
   */
  void cursorAtEnd();
  /**
   * Toggle completion.
   */
  void enableCompletion( bool enable );

protected:
  /**
   * Always call AddressLineEdit::loadAddresses() as the first thing.
   * Use addAddress() to add addresses.
   */
  virtual void loadAddresses();
  void addAddress( const TQString& );
  virtual void keyPressEvent(TQKeyEvent*);
  virtual void dropEvent(TQDropEvent *e);
  virtual void paste();
  virtual void insert(const TQString &t);
  virtual void mouseReleaseEvent( TQMouseEvent * e );
  void doCompletion(bool ctrlT);

private slots:
  void slotCompletion() { doCompletion(false); }
  void slotPopupCompletion( const TQString& );
  void slotStartLDAPLookup();
  void slotLDAPSearchData( const TQStringList& );

private:
  void init();
  void startLoadingLDAPEntries();
  void stopLDAPLookup();
  TQStringList addresses();
  TQStringList removeMailDupes( const TQStringList& adrs );

  TQString m_previousAddresses;
  bool m_useCompletion;
  bool m_completionInitialized;
  bool m_smartPaste;
  TQString m_typedText; // unused

  static bool s_addressesDirty;
  static TDECompletion *s_completion;
  static TQTimer *s_LDAPTimer;
  static LdapSearch *s_LDAPSearch;
  static TQString *s_LDAPText;
  static AddressLineEdit *s_LDAPLineEdit;
  static TDEConfig *s_config;

private:
  class AddressLineEditPrivate* d;
};

}

#endif		/* KABC_ADDRESSLINEEDIT_H */
