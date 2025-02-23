/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KTEXTEDIT_H
#define KTEXTEDIT_H

#include <tqtextedit.h>

#include <tdelibs_export.h>

class KSpell;

/**
 * @short A KDE'ified QTextEdit
 *
 * This is just a little subclass of TQTextEdit, implementing
 * some standard KDE features, like Cursor auto-hiding, configurable
 * wheelscrolling (fast-scroll or zoom) and deleting of entire
 * words with Ctrl-Backspace or Ctrl-Delete.
 *
 * Basic rule: whenever you want to use TQTextEdit, use KTextEdit!
 *
 * @see QTextEdit
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @since 3.1
 */
class TDEUI_EXPORT KTextEdit : public TQTextEdit
{
    TQ_OBJECT

public:
    /**
     * Constructs a KTextEdit object. See TQTextEdit::QTextEdit
     * for details.
     */
    KTextEdit( const TQString& text,
               const TQString& context = TQString::null,
               TQWidget * parent = 0, const char *name = 0 );
    /**
     * Constructs a KTextEdit object. See TQTextEdit::QTextEdit
     * for details.
     */

    KTextEdit( TQWidget *parent = 0L, const char *name = 0 );
    /**
     * Destroys the KTextEdit object.
     */
    ~KTextEdit();

    /**
     * Reimplemented to set a proper "deactivated" background color.
     */
    virtual void setReadOnly (bool readOnly);

    /**
     * Reimplemented for tracking custom palettes.
     */
    virtual void setPalette( const TQPalette& palette );

    /**
     * Turns spell checking for this text edit on or off. Note that spell
     * checking is only available in read-writable KTextEdits.
     *
     * @see checkSpellingEnabled()
     * @see isReadOnly()
     * @see setReadOnly()
     * @since 3.2
     */
    void setCheckSpellingEnabled( bool check );

    /**
     * Returns true if spell checking is enabled for this text edit.
     * Note that it even returns true if this is a read-only KTextEdit,
     * where spell checking is actually disabled.
     *
     @ see setCheckSpellingEnabled()
     * @since 3.2
     */
    bool checkSpellingEnabled() const;

    void highLightWord( unsigned int length, unsigned int pos );


public slots:
    /**
     * Create a modal dialog to check the spelling.  This slot will not return
     * until spell checking has been completed.
     *
     * @since 3.2
     */
    void checkSpelling();

protected:
    /**
     * Reimplemented to catch "delete word" key events.
     */
    virtual void keyPressEvent( TQKeyEvent * );

    /**
     * Reimplemented to instantiate a KDictSpellingHighlighter, if
     * spellchecking is enabled.
     */
    virtual void focusInEvent( TQFocusEvent * );

    /**
     * Reimplemented to allow fast-wheelscrolling with Ctrl-Wheel
     * or zoom.
     */
    virtual void contentsWheelEvent( TQWheelEvent * );

    /**
     * Deletes a word backwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordBack();

    /**
     * Deletes a word forwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordForward();

    /**
     * Reimplemented from TQTextEdit to add spelling related items
     * when appropriate.
     */
    virtual TQPopupMenu *createPopupMenu( const TQPoint &pos );

    /**
     * This is just a reimplementation of a deprecated method from TQTextEdit and
     * is just here to keep source compatibility.  This should not be used in
     * new code.  Specifically reimplementing this method will probably not do
     * what you expect.  See the method above.
     *
     * @deprecated
     */
    virtual TQPopupMenu *createPopupMenu();

protected:
    virtual void virtual_hook( int id, void* data );

private slots:
    void slotSpellCheckReady( KSpell *s );
    void slotSpellCheckDone( const TQString &s );
    void spellCheckerMisspelling( const TQString &text, const TQStringList &, unsigned int pos);
    void spellCheckerCorrected( const TQString &, const TQString &, unsigned int );
    void spellCheckerFinished();
    void toggleAutoSpellCheck();
    void slotAllowTab();

private:
    void posToRowCol( unsigned int pos, unsigned int &line, unsigned int &col );

    class KTextEditPrivate;
    KTextEditPrivate *d;
};

#endif // KTEXTEDIT_H
