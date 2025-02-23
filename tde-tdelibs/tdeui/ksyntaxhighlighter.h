/*
 ksyntaxhighlighter.cpp

 Copyright (c) 2003 Trolltech AS
 Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

 This file is part of the KDE libraries

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

#ifndef KSYNTAXHIGHLIGHTER_H
#define KSYNTAXHIGHLIGHTER_H

#include <tqtextedit.h>
#include <tqsyntaxhighlighter.h>
#include <tqcolor.h>
#include <tqstringlist.h>

#include <tdelibs_export.h>

class TQAccel;
class TQTimer;
class KSpell;
class KSpellConfig;

/**
 * \brief Syntax sensitive text highlighter
 */
class TDEUI_EXPORT KSyntaxHighlighter : public TQSyntaxHighlighter
{
public:
    enum SyntaxMode {
	PlainTextMode,
	RichTextMode
    };
    KSyntaxHighlighter( TQTextEdit *textEdit,
			 bool colorQuoting = false,
			 const TQColor& QuoteColor0 = black,
			 const TQColor& QuoteColor1 = TQColor( 0x00, 0x80, 0x00 ),
			 const TQColor& QuoteColor2 = TQColor( 0x00, 0x80, 0x00 ),
			 const TQColor& QuoteColor3 = TQColor( 0x00, 0x80, 0x00 ),
			 SyntaxMode mode = PlainTextMode );
    ~KSyntaxHighlighter();

    int highlightParagraph( const TQString& text, int endStateOfLastPara );

private:
    class KSyntaxHighlighterPrivate;
    KSyntaxHighlighterPrivate *d;
};

class TDEUI_EXPORT KSpellingHighlighter : public KSyntaxHighlighter
{
public:
    KSpellingHighlighter( TQTextEdit *textEdit,
			  const TQColor& spellColor = red,
			  bool colorQuoting = false,
			  const TQColor& QuoteColor0 = black,
			  const TQColor& QuoteColor1 = TQColor( 0x00, 0x80, 0x00 ),
			  const TQColor& QuoteColor2 = TQColor( 0x00, 0x80, 0x00 ),
			  const TQColor& QuoteColor3 = TQColor( 0x00, 0x80, 0x00 ) );
    ~KSpellingHighlighter();

    virtual int highlightParagraph( const TQString &text,
				    int endStateOfLastPara );
    virtual bool isMisspelled( const TQString& word ) = 0;
    bool intraWordEditing() const;
    void setIntraWordEditing( bool editing );
    static TQStringList personalWords();

private:
    void flushCurrentWord();

    class KSpellingHighlighterPrivate;
    KSpellingHighlighterPrivate *d;
};

/**
 * \brief Dictionary sensitive text highlighter
 */
class TDEUI_EXPORT KDictSpellingHighlighter : public TQObject, public KSpellingHighlighter
{
TQ_OBJECT

public:
    KDictSpellingHighlighter( TQTextEdit *textEdit,
			      bool spellCheckingActive = true,
			      bool autoEnable = true,
			      const TQColor& spellColor = red,
			      bool colorQuoting = false,
			      const TQColor& QuoteColor0 = black,
			      const TQColor& QuoteColor1 = TQColor( 0x00, 0x80, 0x00 ),
			      const TQColor& QuoteColor2 = TQColor( 0x00, 0x70, 0x00 ),
			      const TQColor& QuoteColor3 = TQColor( 0x00, 0x60, 0x00 ),
                              KSpellConfig *spellConfig = 0 );
    ~KDictSpellingHighlighter();

    virtual bool isMisspelled( const TQString &word );
    static void dictionaryChanged();
    void restartBackgroundSpellCheck();

    /**
     * @short Enable/Disable spell checking.
     *
     * If @p active is true then spell checking is enabled; otherwise it
     * is disabled. Note that you have to disable automatic (de)activation
     * with @ref setAutomatic() before you change the state of spell checking
     * if you want to persistently enable/disable spell checking.
     *
     * @param active if true, then spell checking is enabled
     *
     * @see isActive(), setAutomatic()
     */
    void setActive( bool active );

    /**
     * Returns the state of spell checking.
     *
     * @return true if spell checking is active
     *
     * @see setActive()
     */
    bool isActive() const;

    /**
     * @short En-/Disable automatic (de)activation in case of too many errors.
     *
     * If @p automatic is true then spell checking will be deactivated if
     * too many words were mispelled and spell checking will be activated
     * again if the amount of mispelled words drop below a certain threshold.
     *
     * @param automatic if true, then automatic (de)activation is enabled
     *
     * @see automatic()
     */
    void setAutomatic( bool automatic );

    /**
     * Returns the state of automatic (de)activation.
     *
     * @return true if automatic (de)activation is enabled
     *
     * @see setAutomatic()
     */
    bool automatic() const;

signals:
    void activeChanged(const TQString &);
    void newSuggestions(const TQString& originalword, const TQStringList& suggestions,
                        unsigned int pos);

protected:
    TQString spellKey();
    bool eventFilter(TQObject *o, TQEvent *e);

protected slots:
    void slotMisspelling( const TQString &originalWord, const TQStringList &suggestions, unsigned int pos );
    void slotCorrected( const TQString &originalWord, const TQString &, unsigned int );
    void slotRehighlight();
    void slotDictionaryChanged();
    void slotSpellReady( KSpell *spell );
    void slotAutoDetection();
    void slotLocalSpellConfigChanged();
    void slotKSpellNotResponding();

private:
    class KDictSpellingHighlighterPrivate;
    KDictSpellingHighlighterPrivate *d;
};

#endif
