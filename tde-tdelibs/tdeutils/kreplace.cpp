/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqlabel.h>
#include <tdeapplication.h>
#include <kdebug.h>

#include <tdelocale.h>
#include <tdemessagebox.h>
#include "kreplace.h"
#include "kreplacedialog.h"
#include <tqregexp.h>

//#define DEBUG_REPLACE
#define INDEX_NOMATCH -1

class KReplaceNextDialog : public KDialogBase
{
public:
    KReplaceNextDialog( TQWidget *parent );
    void setLabel( const TQString& pattern, const TQString& replacement );
private:
    TQLabel* m_mainLabel;
};

KReplaceNextDialog::KReplaceNextDialog(TQWidget *parent) :
    KDialogBase(parent, 0, false,  // non-modal!
        i18n("Replace"),
        User3 | User2 | User1 | Close,
        User3,
        false,
        i18n("&All"), i18n("&Skip"), i18n("Replace"))
{
    m_mainLabel = new TQLabel( this );
    setMainWidget( m_mainLabel );
    resize(minimumSize());
}

void KReplaceNextDialog::setLabel( const TQString& pattern, const TQString& replacement )
{
    m_mainLabel->setText( i18n("Replace '%1' with '%2'?").arg(pattern).arg(replacement) );
}

////

KReplace::KReplace(const TQString &pattern, const TQString &replacement, long options, TQWidget *parent) :
    KFind( pattern, options, parent )
{
    m_replacements = 0;
    m_replacement = replacement;
}

KReplace::KReplace(const TQString &pattern, const TQString &replacement, long options, TQWidget *parent, TQWidget *dlg) :
    KFind( pattern, options, parent, dlg )
{
    m_replacements = 0;
    m_replacement = replacement;
}

KReplace::~KReplace()
{
    // KFind::~KFind will delete m_dialog
}

KDialogBase* KReplace::replaceNextDialog( bool create )
{
    if ( m_dialog || create )
        return dialog();
    return 0L;
}

KReplaceNextDialog* KReplace::dialog()
{
    if ( !m_dialog )
    {
        m_dialog = new KReplaceNextDialog( parentWidget() );
        connect( m_dialog, TQ_SIGNAL( user1Clicked() ), this, TQ_SLOT( slotReplaceAll() ) );
        connect( m_dialog, TQ_SIGNAL( user2Clicked() ), this, TQ_SLOT( slotSkip() ) );
        connect( m_dialog, TQ_SIGNAL( user3Clicked() ), this, TQ_SLOT( slotReplace() ) );
        connect( m_dialog, TQ_SIGNAL( finished() ), this, TQ_SLOT( slotDialogClosed() ) );
    }
    return static_cast<KReplaceNextDialog *>(m_dialog);
}

void KReplace::displayFinalDialog() const
{
    if ( !m_replacements )
        KMessageBox::information(parentWidget(), i18n("No text was replaced."));
    else
        KMessageBox::information(parentWidget(), i18n("1 replacement done.", "%n replacements done.", m_replacements ) );
}

KFind::Result KReplace::replace()
{
#ifdef DEBUG_REPLACE
    kdDebug() << k_funcinfo << "m_index=" << m_index << endl;
#endif
    if ( m_index == INDEX_NOMATCH && m_lastResult == Match )
    {
        m_lastResult = NoMatch;
        return NoMatch;
    }

    do // this loop is only because validateMatch can fail
    {
#ifdef DEBUG_REPLACE
        kdDebug() << k_funcinfo << "beginning of loop: m_index=" << m_index << endl;
#endif
        // Find the next match.
        if ( m_options & KReplaceDialog::RegularExpression )
            m_index = KFind::find(m_text, *m_regExp, m_index, m_options, &m_matchedLength);
        else
            m_index = KFind::find(m_text, m_pattern, m_index, m_options, &m_matchedLength);
#ifdef DEBUG_REPLACE
        kdDebug() << k_funcinfo << "KFind::find returned m_index=" << m_index << endl;
#endif
        if ( m_index != -1 )
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( m_text, m_index, m_matchedLength ) )
            {
                if ( m_options & KReplaceDialog::PromptOnReplace )
                {
#ifdef DEBUG_REPLACE
                    kdDebug() << k_funcinfo << "PromptOnReplace" << endl;
#endif
                    // Display accurate initial string and replacement string, they can vary
                    TQString matchedText = m_text.mid( m_index, m_matchedLength );
                    TQString rep = matchedText;
                    KReplace::replace(rep, m_replacement, 0, m_options, m_matchedLength);
                    dialog()->setLabel( matchedText, rep );
                    dialog()->show();

                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    emit highlight(m_text, m_index, m_matchedLength);

                    m_lastResult = Match;
                    return Match;
                }
                else
                {
                    doReplace(); // this moves on too
                }
            }
            else
            {
                // not validated -> move on
                if (m_options & KFindDialog::FindBackwards)
                    m_index--;
                else
                    m_index++;
            }
        } else
            m_index = INDEX_NOMATCH; // will exit the loop
    }
    while (m_index != INDEX_NOMATCH);

    m_lastResult = NoMatch;
    return NoMatch;
}

int KReplace::replace(TQString &text, const TQString &pattern, const TQString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, options, matchedLength);
        if (options & KReplaceDialog::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KReplace::replace(TQString &text, const TQRegExp &pattern, const TQString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        *replacedLength = replace(text, replacement, index, options, matchedLength);
        if (options & KReplaceDialog::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

int KReplace::replace(TQString &text, const TQString &replacement, int index, long options, int length)
{
    TQString rep = replacement;
    // Backreferences: replace \0 with the right portion of 'text'
    if ( options & KReplaceDialog::BackReference )
        rep.replace( "\\0", text.mid( index, length ) );
    // Then replace rep into the text
    text.replace(index, length, rep);
    return rep.length();
}

void KReplace::slotReplaceAll()
{
    doReplace();
    m_options &= ~KReplaceDialog::PromptOnReplace;
    emit optionsChanged();
    emit findNext();
}

void KReplace::slotSkip()
{
    if (m_options & KReplaceDialog::FindBackwards)
        m_index--;
    else
        m_index++;
    if ( m_dialogClosed ) {
        delete m_dialog; // hide it again
        m_dialog = 0L;
    } else
        emit findNext();
}

void KReplace::slotReplace()
{
    doReplace();
    if ( m_dialogClosed ) {
        delete m_dialog; // hide it again
        m_dialog = 0L;
    } else
        emit findNext();
}

void KReplace::doReplace()
{
    int replacedLength = KReplace::replace(m_text, m_replacement, m_index, m_options, m_matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit replace(m_text, m_index, replacedLength, m_matchedLength);
#ifdef DEBUG_REPLACE
    kdDebug() << k_funcinfo << "after replace() signal: m_index=" << m_index << " replacedLength=" << replacedLength << endl;
#endif
    m_replacements++;
    if (m_options & KReplaceDialog::FindBackwards)
        m_index--;
    else {
        m_index += replacedLength;
        // when replacing the empty pattern, move on. See also kjs/regexp.cpp for how this should be done for regexps.
        if ( m_pattern.isEmpty() )
            ++m_index;
    }
#ifdef DEBUG_REPLACE
    kdDebug() << k_funcinfo << "after adjustement: m_index=" << m_index << endl;
#endif
}

void KReplace::resetCounts()
{
    KFind::resetCounts();
    m_replacements = 0;
}

bool KReplace::shouldRestart( bool forceAsking, bool showNumMatches ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // ... Or if the prompt-on-replace option was set.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (m_options & KFindDialog::FromCursor) == 0
         && (m_options & KReplaceDialog::PromptOnReplace) == 0 )
    {
        displayFinalDialog();
        return false;
    }
    TQString message;
    if ( showNumMatches )
    {
        if ( !m_replacements )
            message = i18n("No text was replaced.");
        else
            message = i18n("1 replacement done.", "%n replacements done.", m_replacements );
    }
    else
    {
        if ( m_options & KFindDialog::FindBackwards )
            message = i18n( "Beginning of document reached." );
        else
            message = i18n( "End of document reached." );
    }

    message += "\n";
    // Hope this word puzzle is ok, it's a different sentence
    message +=
        ( m_options & KFindDialog::FindBackwards ) ?
        i18n("Do you want to restart search from the end?")
        : i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), message, TQString::null, i18n("Restart"), i18n("Stop") );
    return( ret == KMessageBox::Yes );
}

void KReplace::closeReplaceNextDialog()
{
    closeFindNextDialog();
}

#include "kreplace.moc"
