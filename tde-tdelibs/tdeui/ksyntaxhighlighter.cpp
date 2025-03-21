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

#include <tqcolor.h>
#include <tqregexp.h>
#include <tqsyntaxhighlighter.h>
#include <tqtimer.h>

#include <tdelocale.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdespell.h>
#include <tdeapplication.h>

#include "ksyntaxhighlighter.h"

static int dummy, dummy2, dummy3, dummy4;
static int *Okay = &dummy;
static int *NotOkay = &dummy2;
static int *Ignore = &dummy3;
static int *Unknown = &dummy4;
static const int tenSeconds = 10*1000;

class KSyntaxHighlighter::KSyntaxHighlighterPrivate
{
public:
    TQColor col1, col2, col3, col4, col5;
    SyntaxMode mode;
    bool enabled;
};

class KSpellingHighlighter::KSpellingHighlighterPrivate
{
public:

    KSpellingHighlighterPrivate() :
	alwaysEndsWithSpace( true ),
	intraWordEditing( false ) {}

    TQString currentWord;
    int currentPos;
    bool alwaysEndsWithSpace;
    TQColor color;
    bool intraWordEditing;
};

class KDictSpellingHighlighter::KDictSpellingHighlighterPrivate
{
public:
    KDictSpellingHighlighterPrivate() :
        mDict( 0 ),
	spell( 0 ),
        mSpellConfig( 0 ),
        rehighlightRequest( 0 ),
	wordCount( 0 ),
	errorCount( 0 ),
	autoReady( false ),
        globalConfig( true ),
	spellReady( false ) {}

    ~KDictSpellingHighlighterPrivate() {
	delete rehighlightRequest;
	delete spell;
    }

    static TQDict<int>* sDict()
    {
	if (!statDict)
	    statDict = new TQDict<int>(50021);
	return statDict;
    }

    TQDict<int>* mDict;
    TQDict<int> autoDict;
    TQDict<int> autoIgnoreDict;
    static TQObject *sDictionaryMonitor;
    KSpell *spell;
    KSpellConfig *mSpellConfig;
    TQTimer *rehighlightRequest, *spellTimeout;
    TQString spellKey;
    int wordCount, errorCount;
    int checksRequested, checksDone;
    int disablePercentage;
    int disableWordCount;
    bool completeRehighlightRequired;
    bool active, automatic, autoReady;
    bool globalConfig, spellReady;
private:
    static TQDict<int>* statDict;

};

TQDict<int>* KDictSpellingHighlighter::KDictSpellingHighlighterPrivate::statDict = 0;


KSyntaxHighlighter::KSyntaxHighlighter( TQTextEdit *textEdit,
					  bool colorQuoting,
					  const TQColor& depth0,
					  const TQColor& depth1,
					  const TQColor& depth2,
					  const TQColor& depth3,
					  SyntaxMode mode )
    : TQSyntaxHighlighter( textEdit )
{
    d = new KSyntaxHighlighterPrivate();

    d->enabled = colorQuoting;
    d->col1 = depth0;
    d->col2 = depth1;
    d->col3 = depth2;
    d->col4 = depth3;
    d->col5 = depth0;

    d->mode = mode;
}

KSyntaxHighlighter::~KSyntaxHighlighter()
{
    delete d;
}

int KSyntaxHighlighter::highlightParagraph( const TQString &text, int )
{
    if (!d->enabled) {
	setFormat( 0, text.length(), textEdit()->viewport()->paletteForegroundColor() );
	return 0;
    }

    TQString simplified = text;
    simplified = TQString(simplified.replace( TQRegExp( "\\s" ), TQString() )).replace( '|', TQString::fromLatin1(">") );
    while ( simplified.startsWith( TQString::fromLatin1(">>>>") ) )
	simplified = simplified.mid(3);
    if	( simplified.startsWith( TQString::fromLatin1(">>>") ) || simplified.startsWith( TQString::fromLatin1("> >	>") ) )
	setFormat( 0, text.length(), d->col2 );
    else if	( simplified.startsWith( TQString::fromLatin1(">>") ) || simplified.startsWith( TQString::fromLatin1("> >") ) )
	setFormat( 0, text.length(), d->col3 );
    else if	( simplified.startsWith( TQString::fromLatin1(">") ) )
	setFormat( 0, text.length(), d->col4 );
    else
	setFormat( 0, text.length(), d->col5 );
    return 0;
}

KSpellingHighlighter::KSpellingHighlighter( TQTextEdit *textEdit,
					    const TQColor& spellColor,
					    bool colorQuoting,
					    const TQColor& depth0,
					    const TQColor& depth1,
					    const TQColor& depth2,
					    const TQColor& depth3 )
    : KSyntaxHighlighter( textEdit, colorQuoting, depth0, depth1, depth2, depth3 )
{
    d = new KSpellingHighlighterPrivate();

    d->color = spellColor;
}

KSpellingHighlighter::~KSpellingHighlighter()
{
    delete d;
}

int KSpellingHighlighter::highlightParagraph( const TQString &text,
					      int paraNo )
{
    if ( paraNo == -2 )
	paraNo = 0;
    // leave #includes, diffs, and quoted replies alone
    TQString diffAndCo( ">|" );

    bool isCode = diffAndCo.find(text[0]) != -1;

    if ( !text.endsWith(" ") )
	d->alwaysEndsWithSpace = false;

    KSyntaxHighlighter::highlightParagraph( text, -2 );

    if ( !isCode ) {
        int para, index;
	textEdit()->getCursorPosition( &para, &index );
	int len = text.length();
	if ( d->alwaysEndsWithSpace )
	    len--;

	d->currentPos = 0;
	d->currentWord = "";
	for ( int i = 0; i < len; i++ ) {
	    if ( !text[i].isLetter() && (!(text[i] == '\'')) ) {
		if ( ( para != paraNo ) ||
		    !intraWordEditing() ||
		    ( i - d->currentWord.length() > (uint)index ) ||
		    ( i < index ) ) {
		    flushCurrentWord();
		} else {
		    d->currentWord = "";
		}
		d->currentPos = i + 1;
	    } else {
		d->currentWord += text[i];
	    }
	}
	if ( !text[len - 1].isLetter() ||
	     (uint)( index + 1 ) != text.length() ||
	     para != paraNo )
	    flushCurrentWord();
    }
    return ++paraNo;
}

TQStringList KSpellingHighlighter::personalWords()
{
    TQStringList l;
    l.append( "KMail" );
    l.append( "KOrganizer" );
    l.append( "KAddressBook" );
    l.append( "TDEHTML" );
    l.append( "TDEIO" );
    l.append( "KJS" );
    l.append( "Konqueror" );
    l.append( "KSpell" );
    l.append( "Kontact" );
    l.append( "Qt" );
    return l;
}

void KSpellingHighlighter::flushCurrentWord()
{
    while ( d->currentWord[0].isPunct() ) {
	d->currentWord = d->currentWord.mid( 1 );
	d->currentPos++;
    }

    TQChar ch;
    while ( ( ch = d->currentWord[(int) d->currentWord.length() - 1] ).isPunct() &&
	     ch != '(' && ch != '@' )
	d->currentWord.truncate( d->currentWord.length() - 1 );

    if ( !d->currentWord.isEmpty() ) {
	if ( isMisspelled( d->currentWord ) ) {
	    setFormat( d->currentPos, d->currentWord.length(), d->color );
//	    setMisspelled( d->currentPos, d->currentWord.length(), true );
	}
    }
    d->currentWord = "";
}

TQObject *KDictSpellingHighlighter::KDictSpellingHighlighterPrivate::sDictionaryMonitor = 0;

KDictSpellingHighlighter::KDictSpellingHighlighter( TQTextEdit *textEdit,
						    bool spellCheckingActive ,
						    bool autoEnable,
						    const TQColor& spellColor,
						    bool colorQuoting,
						    const TQColor& depth0,
						    const TQColor& depth1,
						    const TQColor& depth2,
						    const TQColor& depth3,
                                                    KSpellConfig *spellConfig )
    : KSpellingHighlighter( textEdit, spellColor,
			    colorQuoting, depth0, depth1, depth2, depth3 )
{
    d = new KDictSpellingHighlighterPrivate();

    d->mSpellConfig = spellConfig;
    d->globalConfig = ( !spellConfig );
    d->automatic = autoEnable;
    d->active = spellCheckingActive;
    d->checksRequested = 0;
    d->checksDone = 0;
    d->completeRehighlightRequired = false;

    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cs( config, "KSpell" );
    d->disablePercentage = config->readNumEntry( "KSpell_AsYouTypeDisablePercentage", 42 );
    d->disablePercentage = TQMIN( d->disablePercentage, 101 );
    d->disableWordCount = config->readNumEntry( "KSpell_AsYouTypeDisableWordCount", 100 );

    textEdit->installEventFilter( this );
    textEdit->viewport()->installEventFilter( this );

    d->rehighlightRequest = new TQTimer(this);
    connect( d->rehighlightRequest, TQ_SIGNAL( timeout() ),
	     this, TQ_SLOT( slotRehighlight() ));
    d->spellTimeout = new TQTimer(this);
    connect( d->spellTimeout, TQ_SIGNAL( timeout() ),
	     this, TQ_SLOT( slotKSpellNotResponding() ));

    if ( d->globalConfig ) {
        d->spellKey = spellKey();

        if ( !d->sDictionaryMonitor )
            d->sDictionaryMonitor = new TQObject();
    }
    else {
        d->mDict = new TQDict<int>(4001);
        connect( d->mSpellConfig, TQ_SIGNAL( configChanged() ),
                 this, TQ_SLOT( slotLocalSpellConfigChanged() ) );
    }

    slotDictionaryChanged();
    // whats this good for?
    //startTimer( 2 * 1000 );
}

KDictSpellingHighlighter::~KDictSpellingHighlighter()
{
    delete d->spell;
    d->spell = 0;
    delete d->mDict;
    d->mDict = 0;
    delete d;
}

void KDictSpellingHighlighter::slotSpellReady( KSpell *spell )
{
    kdDebug(0) << "KDictSpellingHighlighter::slotSpellReady( " << spell << " )" << endl;
    TDEConfigGroup cg( TDEGlobal::config(),"KSpell" );
    if ( cg.readEntry("KSpell_DoSpellChecking") != "0" )
    {
      if ( d->globalConfig ) {
          connect( d->sDictionaryMonitor, TQ_SIGNAL( destroyed()),
                   this, TQ_SLOT( slotDictionaryChanged() ));
      }
      if ( spell != d->spell )
      {
          delete d->spell;
          d->spell = spell;
      }
      d->spellReady = true;
      const TQStringList l = KSpellingHighlighter::personalWords();
      for ( TQStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
          d->spell->addPersonal( *it );
      }
      connect( spell, TQ_SIGNAL( misspelling( const TQString &, const TQStringList &, unsigned int )),
	       this, TQ_SLOT( slotMisspelling( const TQString &, const TQStringList &, unsigned int )));
      connect( spell, TQ_SIGNAL( corrected( const TQString &, const TQString &, unsigned int )),
               this, TQ_SLOT( slotCorrected( const TQString &, const TQString &, unsigned int )));
      d->checksRequested = 0;
      d->checksDone = 0;
      d->completeRehighlightRequired = true;
      d->rehighlightRequest->start( 0, true );
    }
}

bool KDictSpellingHighlighter::isMisspelled( const TQString &word )
{
    if (!d->spellReady)
	return false;

    // This debug is expensive, only enable it locally
    //kdDebug(0) << "KDictSpellingHighlighter::isMisspelled( \"" << word << "\" )" << endl;
    // Normally isMisspelled would look up a dictionary and return
    // true or false, but tdespell is asynchronous and slow so things
    // get tricky...
    // For auto detection ignore signature and reply prefix
    if ( !d->autoReady )
	d->autoIgnoreDict.replace( word, Ignore );

    // "dict" is used as a cache to store the results of KSpell
    TQDict<int>* dict = ( d->globalConfig ? d->sDict() : d->mDict );
    if ( !dict->isEmpty() && (*dict)[word] == NotOkay ) {
	if ( d->autoReady && ( d->autoDict[word] != NotOkay )) {
	    if ( !d->autoIgnoreDict[word] )
		++d->errorCount;
	    d->autoDict.replace( word, NotOkay );
	}

	return d->active;
    }
    if ( !dict->isEmpty() && (*dict)[word] == Okay ) {
	if ( d->autoReady && !d->autoDict[word] ) {
	    d->autoDict.replace( word, Okay );
	}
	return false;
    }

    if ((dict->isEmpty() || !((*dict)[word])) && d->spell ) {
	int para, index;
	textEdit()->getCursorPosition( &para, &index );
	++d->wordCount;
	dict->replace( word, Unknown );
	++d->checksRequested;
	if (currentParagraph() != para)
	    d->completeRehighlightRequired = true;
	d->spellTimeout->start( tenSeconds, true );
	d->spell->checkWord( word, false );
    }
    return false;
}

bool KSpellingHighlighter::intraWordEditing() const
{
    return d->intraWordEditing;
}

void KSpellingHighlighter::setIntraWordEditing( bool editing )
{
    d->intraWordEditing = editing;
}

void KDictSpellingHighlighter::slotMisspelling (const TQString &originalWord, const TQStringList &suggestions,
                                                unsigned int pos)
{
    Q_UNUSED( suggestions );
    // kdDebug() << suggestions.join( " " ).latin1() << endl;
    if ( d->globalConfig )
        d->sDict()->replace( originalWord, NotOkay );
    else
        d->mDict->replace( originalWord, NotOkay );

    //Emit this baby so that apps that want to have suggestions in a popup over
    //the misspelled word can catch them.
    emit newSuggestions( originalWord, suggestions, pos );
}

void KDictSpellingHighlighter::slotCorrected(const TQString &word,
					     const TQString &,
					     unsigned int)

{
    TQDict<int>* dict = ( d->globalConfig ? d->sDict() : d->mDict );
    if ( !dict->isEmpty() && (*dict)[word] == Unknown ) {
        dict->replace( word, Okay );
    }
    ++d->checksDone;
    if (d->checksDone == d->checksRequested) {
	d->spellTimeout->stop();
      slotRehighlight();
    } else {
	d->spellTimeout->start( tenSeconds, true );
    }
}

void KDictSpellingHighlighter::dictionaryChanged()
{
    TQObject *oldMonitor = KDictSpellingHighlighterPrivate::sDictionaryMonitor;
    KDictSpellingHighlighterPrivate::sDictionaryMonitor = new TQObject();
    KDictSpellingHighlighterPrivate::sDict()->clear();
    delete oldMonitor;
}

void KDictSpellingHighlighter::restartBackgroundSpellCheck()
{
    kdDebug(0) << "KDictSpellingHighlighter::restartBackgroundSpellCheck()" << endl;
    slotDictionaryChanged();
}

void KDictSpellingHighlighter::setActive( bool active )
{
    if ( active == d->active )
        return;

    d->active = active;
    rehighlight();
    if ( d->active )
        emit activeChanged( i18n("As-you-type spell checking enabled.") );
    else
        emit activeChanged( i18n("As-you-type spell checking disabled.") );
}

bool KDictSpellingHighlighter::isActive() const
{
    return d->active;
}

void KDictSpellingHighlighter::setAutomatic( bool automatic )
{
    if ( automatic == d->automatic )
        return;

    d->automatic = automatic;
    if ( d->automatic )
        slotAutoDetection();
}

bool KDictSpellingHighlighter::automatic() const
{
    return d->automatic;
}

void KDictSpellingHighlighter::slotRehighlight()
{
    kdDebug(0) << "KDictSpellingHighlighter::slotRehighlight()" << endl;
    if (d->completeRehighlightRequired) {
	rehighlight();
    } else {
	int para, index;
	textEdit()->getCursorPosition( &para, &index );
	//rehighlight the current para only (undo/redo safe)
	bool modified = textEdit()->isModified();
	textEdit()->insertAt( "", para, index );
	textEdit()->setModified( modified );
    }
    if (d->checksDone == d->checksRequested)
	d->completeRehighlightRequired = false;
    TQTimer::singleShot( 0, this, TQ_SLOT( slotAutoDetection() ));
}

void KDictSpellingHighlighter::slotDictionaryChanged()
{
    delete d->spell;
    d->spellReady = false;
    d->wordCount = 0;
    d->errorCount = 0;
    d->autoDict.clear();

    d->spell = new KSpell( 0, i18n( "Incremental Spellcheck" ), this,
		TQ_SLOT( slotSpellReady( KSpell * ) ), d->mSpellConfig );
}

void KDictSpellingHighlighter::slotLocalSpellConfigChanged()
{
    kdDebug(0) << "KDictSpellingHighlighter::slotSpellConfigChanged()" << endl;
    // the spell config has been changed, so we have to restart from scratch
    d->mDict->clear();
    slotDictionaryChanged();
}

TQString KDictSpellingHighlighter::spellKey()
{
    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cs( config, "KSpell" );
    config->reparseConfiguration();
    TQString key;
    key += TQString::number( config->readNumEntry( "KSpell_NoRootAffix", 0 ));
    key += '/';
    key += TQString::number( config->readNumEntry( "KSpell_RunTogether", 0 ));
    key += '/';
    key += config->readEntry( "KSpell_Dictionary", "" );
    key += '/';
    key += TQString::number( config->readNumEntry( "KSpell_DictFromList", false ));
    key += '/';
    key += TQString::number( config->readNumEntry( "KSpell_Encoding", KS_E_UTF8 ));
    key += '/';
    key += TQString::number( config->readNumEntry( "KSpell_Client", KS_CLIENT_ISPELL ));
    return key;
}


// Automatic spell checking support
// In auto spell checking mode disable as-you-type spell checking
// iff more than one third of words are spelt incorrectly.
//
// Words in the signature and reply prefix are ignored.
// Only unique words are counted.

void KDictSpellingHighlighter::slotAutoDetection()
{
    if ( !d->autoReady )
	return;

    bool savedActive = d->active;

    if ( d->automatic ) {
	// tme = Too many errors
	bool tme = d->wordCount >= d->disableWordCount && d->errorCount * 100 >= d->disablePercentage * d->wordCount;
	if ( d->active && tme )
	    d->active = false;
	else if ( !d->active && !tme )
	    d->active = true;
    }
    if ( d->active != savedActive ) {
	if ( d->wordCount > 1 )
	    if ( d->active )
		emit activeChanged( i18n("As-you-type spell checking enabled.") );
	    else
		emit activeChanged( i18n( "Too many misspelled words. "
					  "As-you-type spell checking disabled." ) );
	d->completeRehighlightRequired = true;
	d->rehighlightRequest->start( 100, true );
    }
}

void KDictSpellingHighlighter::slotKSpellNotResponding()
{
    static int retries = 0;
    if (retries < 10) {
        if ( d->globalConfig )
	    KDictSpellingHighlighter::dictionaryChanged();
	else
	    slotLocalSpellConfigChanged();
    } else {
	setAutomatic( false );
	setActive( false );
    }
    ++retries;
}

bool KDictSpellingHighlighter::eventFilter( TQObject *o, TQEvent *e)
{
    if (o == textEdit() && (e->type() == TQEvent::FocusIn)) {
        if ( d->globalConfig ) {
            TQString skey = spellKey();
            if ( d->spell && d->spellKey != skey ) {
                d->spellKey = skey;
                KDictSpellingHighlighter::dictionaryChanged();
            }
        }
    }

    if (o == textEdit() && (e->type() == TQEvent::KeyPress)) {
	TQKeyEvent *k = static_cast<TQKeyEvent*>(e);
	d->autoReady = true;
	if (d->rehighlightRequest->isActive()) // try to stay out of the users way
	    d->rehighlightRequest->changeInterval( 500 );
	if ( k->key() == Key_Enter ||
	     k->key() == Key_Return ||
	     k->key() == Key_Up ||
	     k->key() == Key_Down ||
	     k->key() == Key_Left ||
	     k->key() == Key_Right ||
	     k->key() == Key_PageUp ||
	     k->key() == Key_PageDown ||
	     k->key() == Key_Home ||
	     k->key() == Key_End ||
	     (( k->state() & ControlButton ) &&
	      ((k->key() == Key_A) ||
	       (k->key() == Key_B) ||
	       (k->key() == Key_E) ||
	       (k->key() == Key_N) ||
	       (k->key() == Key_P))) ) {
	    if ( intraWordEditing() ) {
		setIntraWordEditing( false );
		d->completeRehighlightRequired = true;
		d->rehighlightRequest->start( 500, true );
	    }
	    if (d->checksDone != d->checksRequested) {
		// Handle possible change of paragraph while
		// words are pending spell checking
		d->completeRehighlightRequired = true;
		d->rehighlightRequest->start( 500, true );
	    }
	} else {
	    setIntraWordEditing( true );
	}
	if ( k->key() == Key_Space ||
	     k->key() == Key_Enter ||
	     k->key() == Key_Return ) {
	    TQTimer::singleShot( 0, this, TQ_SLOT( slotAutoDetection() ));
	}
    }

    else if ( o == textEdit()->viewport() &&
	 ( e->type() == TQEvent::MouseButtonPress )) {
	d->autoReady = true;
	if ( intraWordEditing() ) {
	    setIntraWordEditing( false );
	    d->completeRehighlightRequired = true;
	    d->rehighlightRequest->start( 0, true );
	}
    }

    return false;
}

#include "ksyntaxhighlighter.moc"
