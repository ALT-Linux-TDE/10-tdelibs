/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith  <dsmith@algonet.se>

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

#ifndef KSHELLCOMPLETION_H
#define KSHELLCOMPLETION_H

#include <tqstring.h>
#include <tqstringlist.h>

#include "kurlcompletion.h"

class KShellCompletionPrivate;

/**
 * This class does shell-like completion of file names.
 * A string passed to makeCompletion() will be interpreted as a shell 
 * command line. Completion will be done on the last argument on the line. 
 * Returned matches consist of the first arguments (uncompleted) plus the 
 * completed last argument.
 *
 * @short Shell-like completion of file names
 * @author David Smith <dsmith@algonet.se>
 */
class TDEIO_EXPORT KShellCompletion : public KURLCompletion 
{
	TQ_OBJECT

public:
        /**
         * Constructs a KShellCompletion object.
         */
	KShellCompletion();

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
	// Find the part of text that should be completed
	void splitText(const TQString &text, TQString &text_start, TQString &text_compl) const;
	// Insert quotes and neseccary escapes
	bool quoteText(TQString *text, bool force, bool skip_last) const;
	TQString unquote(const TQString &text) const;
		                                                                        
	TQString m_text_start; // part of the text that was not completed
	TQString m_text_compl; // part of the text that was completed (unchanged)

    TQChar m_word_break_char;
	TQChar m_quote_char1;
	TQChar m_quote_char2;
	TQChar m_escape_char;  
				
protected:
	virtual void virtual_hook( int id, void* data );
private:
	KShellCompletionPrivate *d;
};

#endif // KSHELLCOMPLETION_H
