/* This file is part of the KDE libraries
   Copyright (C) 2001, 2003 Christoph Cullmann <cullmann@kde.org>

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

#ifndef _KATE_CMD_H
#define _KATE_CMD_H

#include "document.h"

#include <kcompletion.h>

#include <tqdict.h>
#include <tqstringlist.h>

class KATEPARTINTERFACES_EXPORT KateCmd
{
  private:
    KateCmd ();

  public:
    ~KateCmd ();

    static KateCmd *self ();

    bool registerCommand (Kate::Command *cmd);
    bool unregisterCommand (Kate::Command *cmd);
    Kate::Command *queryCommand (const TQString &cmd);

    TQStringList cmds ();
    void appendHistory( const TQString &cmd );
    const TQString fromHistory( uint i ) const;
    uint historyLength() const { return m_history.count(); }

  private:
    static KateCmd *s_self;
    TQDict<Kate::Command> m_dict;
    TQStringList m_cmds;
    TQStringList m_history;
};

/**
 * A TDECompletion object that completes last ?unquoted? word in the string
 * passed. Dont mistake "shell" for anything related to quoting, this
 * simply mimics shell tab completion by completing the last word in the
 * provided text.
 */
class KATEPARTINTERFACES_EXPORT KateCmdShellCompletion : public TDECompletion
{
  public:
    KateCmdShellCompletion();

    /**
     * Finds completions to the given text.
     * The first match is returned and emitted in the signal match().
     * @param text the text to complete
     * @return the first match, or TQString::null if not found
     */
    TQString makeCompletion(const TQString &text);

  protected:
        // Called by TDECompletion
    void postProcessMatch( TQString *match ) const;
    void postProcessMatches( TQStringList *matches ) const;
    void postProcessMatches( TDECompletionMatches *matches ) const;

  private:
  /**
   * Split text at the last unquoted space
   *
   * @param text_start will be set to the text at the left, including the space
   * @param text_compl Will be set to the text at the right. This is the text to complete.
   */
   void splitText( const TQString &text, TQString &text_start, TQString &text_compl ) const;

   TQChar m_word_break_char;
   TQChar m_quote_char1;
   TQChar m_quote_char2;
   TQChar m_escape_char;

   TQString m_text_start;
   TQString m_text_compl;

};

#endif
