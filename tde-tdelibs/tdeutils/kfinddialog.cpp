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

#include "kfinddialog.h"
#include <tqcheckbox.h>
#include <tqcursor.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tqpushbutton.h>
#include <tqregexp.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <assert.h>
#include <tqwhatsthis.h>

#include <kregexpeditorinterface.h>
#include <tdeparts/componentfactory.h>

class KFindDialog::KFindDialogPrivate
{
public:
    KFindDialogPrivate() : m_regexpDialog(0),
        m_regexpDialogQueryDone(false),
        m_enabled(WholeWordsOnly | FromCursor | SelectedText | CaseSensitive | FindBackwards | RegularExpression), m_initialShowDone(false) {}
    TQDialog* m_regexpDialog;
    bool m_regexpDialogQueryDone;
    long m_enabled; // uses Options to define which search options are enabled
    bool m_initialShowDone;
    TQStringList findStrings;
    TQString pattern;
};

KFindDialog::KFindDialog(TQWidget *parent, const char *name, long options, const TQStringList &findStrings, bool hasSelection) :
    KDialogBase(parent, name, true, i18n("Find Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0)
{
    d = new KFindDialogPrivate;
    init(false, findStrings, hasSelection);
    setOptions(options);
    setButtonCancel( KStdGuiItem::close() );
}

KFindDialog::KFindDialog(bool modal, TQWidget *parent, const char *name, long options, const TQStringList &findStrings, bool hasSelection) :
    KDialogBase(parent, name, modal, i18n("Find Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0)
{
    d = new KFindDialogPrivate;
    init(false, findStrings, hasSelection);
    setOptions(options);
    setButtonCancel( KStdGuiItem::close() );
}

KFindDialog::KFindDialog(TQWidget *parent, const char *name, bool /*forReplace*/) :
    KDialogBase(parent, name, true, i18n("Replace Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0)
{
    d = new KFindDialogPrivate;
    setButtonCancel( KStdGuiItem::close() );
}

KFindDialog::~KFindDialog()
{
    delete d;
}

TQWidget *KFindDialog::findExtension()
{
    if (!m_findExtension)
    {
      m_findExtension = new TQWidget(m_findGrp);
      m_findLayout->addMultiCellWidget(m_findExtension, 3, 3, 0, 1);
    }

    return m_findExtension;
}

TQStringList KFindDialog::findHistory() const
{
    return m_find->historyItems();
}

void KFindDialog::init(bool forReplace, const TQStringList &findStrings, bool hasSelection)
{
    TQVBoxLayout *topLayout;
    TQGridLayout *optionsLayout;

    // Create common parts of dialog.
    TQWidget *page = new TQWidget(this);
    setMainWidget(page);

    topLayout = new TQVBoxLayout(page);
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( 0 );

    m_findGrp = new TQGroupBox(0, TQt::Vertical, i18n("Find"), page);
    m_findGrp->layout()->setSpacing( KDialog::spacingHint() );
   // m_findGrp->layout()->setMargin( KDialog::marginHint() );
    m_findLayout = new TQGridLayout(m_findGrp->layout());
    m_findLayout->setSpacing( KDialog::spacingHint() );
   // m_findLayout->setMargin( KDialog::marginHint() );

    m_findLabel = new TQLabel(i18n("&Text to find:"), m_findGrp);
    m_find = new KHistoryCombo(true, m_findGrp);
    m_find->setMaxCount(10);
    m_find->setDuplicatesEnabled(false);
    m_regExp = new TQCheckBox(i18n("Regular e&xpression"), m_findGrp);
    m_regExpItem = new TQPushButton(i18n("&Edit..."), m_findGrp);
    m_regExpItem->setEnabled(false);

    m_findLayout->addWidget(m_findLabel, 0, 0);
    m_findLayout->addMultiCellWidget(m_find, 1, 1, 0, 1);
    m_findLayout->addWidget(m_regExp, 2, 0);
    m_findLayout->addWidget(m_regExpItem, 2, 1);
    topLayout->addWidget(m_findGrp);

    m_replaceGrp = new TQGroupBox(0, TQt::Vertical, i18n("Replace With"), page);
    m_replaceGrp->layout()->setSpacing( KDialog::spacingHint() );
  //  m_replaceGrp->layout()->setMargin( KDialog::marginHint() );
    m_replaceLayout = new TQGridLayout(m_replaceGrp->layout());
    m_replaceLayout->setSpacing( KDialog::spacingHint() );
//    m_replaceLayout->setMargin( KDialog::marginHint() );

    m_replaceLabel = new TQLabel(i18n("Replace&ment text:"), m_replaceGrp);
    m_replace = new KHistoryCombo(true, m_replaceGrp);
    m_replace->setMaxCount(10);
    m_replace->setDuplicatesEnabled(false);
    m_backRef = new TQCheckBox(i18n("Use p&laceholders"), m_replaceGrp);
    m_backRef->setEnabled(m_regExp->isChecked());
    m_backRefItem = new TQPushButton(i18n("Insert Place&holder"), m_replaceGrp);
    m_backRefItem->setEnabled(m_regExp->isChecked() && m_backRef->isChecked());

    m_replaceLayout->addWidget(m_replaceLabel, 0, 0);
    m_replaceLayout->addMultiCellWidget(m_replace, 1, 1, 0, 1);
    m_replaceLayout->addWidget(m_backRef, 2, 0);
    m_replaceLayout->addWidget(m_backRefItem, 2, 1);
    topLayout->addWidget(m_replaceGrp);

    m_optionGrp = new TQGroupBox(0, TQt::Vertical, i18n("Options"), page);
    m_optionGrp->layout()->setSpacing(KDialog::spacingHint());
  //  m_optionGrp->layout()->setMargin(KDialog::marginHint());
    optionsLayout = new TQGridLayout(m_optionGrp->layout());
    optionsLayout->setSpacing( KDialog::spacingHint() );
   // optionsLayout->setMargin( KDialog::marginHint() );

    m_caseSensitive = new TQCheckBox(i18n("C&ase sensitive"), m_optionGrp);
    m_wholeWordsOnly = new TQCheckBox(i18n("&Whole words only"), m_optionGrp);
    m_fromCursor = new TQCheckBox(i18n("From c&ursor"), m_optionGrp);
    m_findBackwards = new TQCheckBox(i18n("Find &backwards"), m_optionGrp);
    m_selectedText = new TQCheckBox(i18n("&Selected text"), m_optionGrp);
    setHasSelection( hasSelection );
    // If we have a selection, we make 'find in selection' default
    // and if we don't, then the option has to be unchecked, obviously.
    m_selectedText->setChecked( hasSelection );
    slotSelectedTextToggled( hasSelection );

    m_promptOnReplace = new TQCheckBox(i18n("&Prompt on replace"), m_optionGrp);
    m_promptOnReplace->setChecked( true );

    optionsLayout->addWidget(m_caseSensitive, 0, 0);
    optionsLayout->addWidget(m_wholeWordsOnly, 1, 0);
    optionsLayout->addWidget(m_fromCursor, 2, 0);
    optionsLayout->addWidget(m_findBackwards, 0, 1);
    optionsLayout->addWidget(m_selectedText, 1, 1);
    optionsLayout->addWidget(m_promptOnReplace, 2, 1);
    topLayout->addWidget(m_optionGrp);

    // We delay creation of these until needed.
    m_patterns = 0L;
    m_placeholders = 0L;

    // signals and slots connections
    connect(m_selectedText, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotSelectedTextToggled(bool)));
    connect(m_regExp, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotRegexCheckBoxToggled(bool)));
    connect(m_backRef, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotPlaceholdersCheckBoxToggled(bool)));
    connect(m_regExpItem, TQ_SIGNAL(clicked()), this, TQ_SLOT(showPatterns()));
    connect(m_backRefItem, TQ_SIGNAL(clicked()), this, TQ_SLOT(showPlaceholders()));

    connect(m_find, TQ_SIGNAL(textChanged ( const TQString & )),this, TQ_SLOT(textSearchChanged( const TQString & )));

    // tab order
    setTabOrder(m_find, m_regExp);
    setTabOrder(m_regExp, m_regExpItem);
    setTabOrder(m_regExpItem, m_replace);
    setTabOrder(m_replace, m_backRef);
    setTabOrder(m_backRef, m_backRefItem);
    setTabOrder(m_backRefItem, m_caseSensitive);
    setTabOrder(m_caseSensitive, m_wholeWordsOnly);
    setTabOrder(m_wholeWordsOnly, m_fromCursor);
    setTabOrder(m_fromCursor, m_findBackwards);
    setTabOrder(m_findBackwards, m_selectedText);
    setTabOrder(m_selectedText, m_promptOnReplace);

    // buddies
    m_findLabel->setBuddy(m_find);
    m_replaceLabel->setBuddy(m_replace);

    if (!forReplace)
    {
        m_promptOnReplace->hide();
        m_replaceGrp->hide();
    }

    d->findStrings = findStrings;
    m_find->setFocus();
    enableButtonOK( !pattern().isEmpty() );
    if (forReplace)
    {
      setButtonOK(KGuiItem( i18n("&Replace"), TQString::null,
                    i18n("Start replace"),
                    i18n("<qt>If you press the <b>Replace</b> button, the text you entered "
                         "above is searched for within the document and any occurrence is "
                         "replaced with the replacement text.</qt>")));
    }
    else
    {
      setButtonOK(KGuiItem( i18n("&Find"), "edit-find",
                    i18n("Start searching"),
                    i18n("<qt>If you press the <b>Find</b> button, the text you entered "
                         "above is searched for within the document.</qt>")));
    }

    // QWhatsthis texts
    TQWhatsThis::add ( m_find, i18n(
            "Enter a pattern to search for, or select a previous pattern from "
            "the list.") );
    TQWhatsThis::add ( m_regExp, i18n(
            "If enabled, search for a regular expression.") );
    TQWhatsThis::add ( m_regExpItem, i18n(
            "Click here to edit your regular expression using a graphical editor.") );
    TQWhatsThis::add ( m_replace, i18n(
            "Enter a replacement string, or select a previous one from the list.") );
    TQWhatsThis::add( m_backRef, i18n(
            "<qt>When regular expressions are enabled, you can select part of the searched text by "
            "enclosing it within parenthesis. Placeholders allow you to insert such text in the "
            "replacement string, similar to how backreferences are used in sed. When enabled, "
            "any occurrence of <code><b>\\N</b></code> (where <code><b>N</b></code> "
            "is a integer number, e.g. \\1, \\2, ...), will be replaced with "
            "the corresponding capture (\"parenthesized substring\") from the "
            "pattern.<p>To include a literal <code><b>\\N</b></code> in your "
            "replacement, put an extra backslash in front of it, like "
            "<code><b>\\\\N</b></code>.</qt>") );
    TQWhatsThis::add ( m_backRefItem, i18n(
            "Click for a menu of available captures.") );
    TQWhatsThis::add ( m_wholeWordsOnly, i18n(
            "Require word boundaries in both ends of a match to succeed.") );
    TQWhatsThis::add ( m_fromCursor, i18n(
            "Start searching at the current cursor location rather than at the top.") );
    TQWhatsThis::add ( m_selectedText, i18n(
            "Only search within the current selection.") );
    TQWhatsThis::add ( m_caseSensitive, i18n(
            "Perform a case sensitive search: entering the pattern "
            "'Joe' will not match 'joe' or 'JOE', only 'Joe'.") );
    TQWhatsThis::add ( m_findBackwards, i18n(
            "Search backwards.") );
    TQWhatsThis::add ( m_promptOnReplace, i18n(
            "Ask before replacing each match found.") );
}

void KFindDialog::textSearchChanged(const TQString & text)
{
    enableButtonOK( !text.isEmpty() );
}

void KFindDialog::slotRegexCheckBoxToggled(bool checked)
{
    m_regExpItem->setEnabled(checked);    
    m_backRef->setEnabled(checked);
    m_backRefItem->setEnabled(checked && m_backRef->isChecked());
}

void KFindDialog::slotPlaceholdersCheckBoxToggled(bool checked)
{
    m_backRefItem->setEnabled(checked && m_regExp->isChecked());
}

void KFindDialog::showEvent( TQShowEvent *e )
{
    if ( !d->m_initialShowDone )
    {
        d->m_initialShowDone = true; // only once
        kdDebug() << "showEvent\n";
        if (!d->findStrings.isEmpty())
            setFindHistory(d->findStrings);
        d->findStrings = TQStringList();
        if (!d->pattern.isEmpty()) {
            m_find->lineEdit()->setText( d->pattern );
            m_find->lineEdit()->selectAll();
            d->pattern = TQString::null;
        }
    }
    KDialogBase::showEvent(e);
}

long KFindDialog::options() const
{
    long options = 0;

    if (m_caseSensitive->isChecked())
        options |= CaseSensitive;
    if (m_wholeWordsOnly->isChecked())
        options |= WholeWordsOnly;
    if (m_fromCursor->isChecked())
        options |= FromCursor;
    if (m_findBackwards->isChecked())
        options |= FindBackwards;
    if (m_selectedText->isChecked())
        options |= SelectedText;
    if (m_regExp->isChecked())
        options |= RegularExpression;
    return options;
}

TQString KFindDialog::pattern() const
{
    return m_find->currentText();
}

void KFindDialog::setPattern (const TQString &pattern)
{
    m_find->lineEdit()->setText( pattern );
    m_find->lineEdit()->selectAll();
    d->pattern = pattern;
    kdDebug() << "setPattern " << pattern<<endl;
}

void KFindDialog::setFindHistory(const TQStringList &strings)
{
    if (strings.count() > 0)
    {
        m_find->setHistoryItems(strings, true);
        m_find->lineEdit()->setText( strings.first() );
        m_find->lineEdit()->selectAll();
    }
    else
        m_find->clearHistory();
}

void KFindDialog::setHasSelection(bool hasSelection)
{
    if (hasSelection) d->m_enabled |= SelectedText;
    else d->m_enabled &= ~SelectedText;
    m_selectedText->setEnabled( hasSelection );
    if ( !hasSelection )
    {
        m_selectedText->setChecked( false );
        slotSelectedTextToggled( hasSelection );
    }
}

void KFindDialog::slotSelectedTextToggled(bool selec)
{
    // From cursor doesn't make sense if we have a selection
    m_fromCursor->setEnabled( !selec && (d->m_enabled & FromCursor) );
    if ( selec ) // uncheck if disabled
        m_fromCursor->setChecked( false );
}

void KFindDialog::setHasCursor(bool hasCursor)
{
    if (hasCursor) d->m_enabled |= FromCursor;
    else d->m_enabled &= ~FromCursor;
    m_fromCursor->setEnabled( hasCursor );
    m_fromCursor->setChecked( hasCursor && (options() & FromCursor) );
}

void KFindDialog::setSupportsBackwardsFind( bool supports )
{
    // ########## Shouldn't this hide the checkbox instead?
    if (supports) d->m_enabled |= FindBackwards;
    else d->m_enabled &= ~FindBackwards;
    m_findBackwards->setEnabled( supports );
    m_findBackwards->setChecked( supports && (options() & FindBackwards) );
}

void KFindDialog::setSupportsCaseSensitiveFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->m_enabled |= CaseSensitive;
    else d->m_enabled &= ~CaseSensitive;
    m_caseSensitive->setEnabled( supports );
    m_caseSensitive->setChecked( supports && (options() & CaseSensitive) );
}

void KFindDialog::setSupportsWholeWordsFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->m_enabled |= WholeWordsOnly;
    else d->m_enabled &= ~WholeWordsOnly;
    m_wholeWordsOnly->setEnabled( supports );
    m_wholeWordsOnly->setChecked( supports && (options() & WholeWordsOnly) );
}

void KFindDialog::setSupportsRegularExpressionFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->m_enabled |= RegularExpression;
    else d->m_enabled &= ~RegularExpression;
    m_regExp->setEnabled( supports );
    m_regExp->setChecked( supports && (options() & RegularExpression) );
}

void KFindDialog::setOptions(long options)
{
    m_caseSensitive->setChecked((d->m_enabled & CaseSensitive) && (options & CaseSensitive));
    m_wholeWordsOnly->setChecked((d->m_enabled & WholeWordsOnly) && (options & WholeWordsOnly));
    m_fromCursor->setChecked((d->m_enabled & FromCursor) && (options & FromCursor));
    m_findBackwards->setChecked((d->m_enabled & FindBackwards) && (options & FindBackwards));
    m_selectedText->setChecked((d->m_enabled & SelectedText) && (options & SelectedText));
    m_regExp->setChecked((d->m_enabled & RegularExpression) && (options & RegularExpression));
}

// Create a popup menu with a list of regular expression terms, to help the user
// compose a regular expression search pattern.
void KFindDialog::showPatterns()
{
    if ( !d->m_regexpDialogQueryDone )
    {
        d->m_regexpDialog = KParts::ComponentFactory::createInstanceFromQuery<TQDialog>( "KRegExpEditor/KRegExpEditor", TQString(), this );
        d->m_regexpDialogQueryDone = true;
    }

    if ( d->m_regexpDialog )
    {
        KRegExpEditorInterface *iface = dynamic_cast<KRegExpEditorInterface *>( d->m_regexpDialog );
        assert( iface );

        iface->setRegExp( pattern() );
        if ( d->m_regexpDialog->exec() == TQDialog::Accepted )
            setPattern( iface->regExp() );
    }
    else // No complete regexp-editor available, bring up the old popupmenu
    {
        typedef struct
        {
            const char *description;
            const char *regExp;
            int cursorAdjustment;
        } term;
        static const term items[] =
            {
                { I18N_NOOP("Any Character"),                 ".",        0 },
                { I18N_NOOP("Start of Line"),                 "^",        0 },
                { I18N_NOOP("End of Line"),                   "$",        0 },
                { I18N_NOOP("Set of Characters"),             "[]",       -1 },
                { I18N_NOOP("Repeats, Zero or More Times"),   "*",        0 },
                { I18N_NOOP("Repeats, One or More Times"),    "+",        0 },
                { I18N_NOOP("Optional"),                      "?",        0 },
                { I18N_NOOP("Escape"),                        "\\",       0 },
                { I18N_NOOP("TAB"),                           "\\t",      0 },
                { I18N_NOOP("Newline"),                       "\\n",      0 },
                { I18N_NOOP("Carriage Return"),               "\\r",      0 },
                { I18N_NOOP("White Space"),                   "\\s",      0 },
                { I18N_NOOP("Digit"),                         "\\d",      0 },
            };
        int i;

        // Populate the popup menu.
        if (!m_patterns)
        {
            m_patterns = new TQPopupMenu(this);
            for (i = 0; (unsigned)i < sizeof(items) / sizeof(items[0]); i++)
            {
                m_patterns->insertItem(i18n(items[i].description), i, i);
            }
        }

        // Insert the selection into the edit control.
        i = m_patterns->exec(m_regExpItem->mapToGlobal(m_regExpItem->rect().bottomLeft()));
        if (i != -1)
        {
            TQLineEdit *editor = m_find->lineEdit();

            editor->insert(items[i].regExp);
            editor->setCursorPosition(editor->cursorPosition() + items[i].cursorAdjustment);
        }
    }
}

// Create a popup menu with a list of backreference terms, to help the user
// compose a regular expression replacement pattern.
void KFindDialog::showPlaceholders()
{
    // Populate the popup menu.
    if (!m_placeholders)
    {
        m_placeholders = new TQPopupMenu(this);
        connect( m_placeholders, TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(slotPlaceholdersAboutToShow()) );
    }

    // Insert the selection into the edit control.
    int i = m_placeholders->exec(m_backRefItem->mapToGlobal(m_backRefItem->rect().bottomLeft()));
    if (i != -1)
    {
        TQLineEdit *editor = m_replace->lineEdit();
        editor->insert( TQString("\\%1").arg( i ) );
    }
}

void KFindDialog::slotPlaceholdersAboutToShow()
{
    m_placeholders->clear();
    m_placeholders->insertItem( i18n("Complete Match"), 0 );

    TQRegExp r( pattern() );
    uint n = r.numCaptures();
    for ( uint i=0; i < n; i++ )
        m_placeholders->insertItem( i18n("Captured Text (%1)").arg( i+1 ), i+1 );
}

void KFindDialog::slotOk()
{
    // Nothing to find?
    if (pattern().isEmpty())
    {
        KMessageBox::error(this, i18n("You must enter some text to search for."));
        return;
    }

    if (m_regExp->isChecked())
    {
        // Check for a valid regular expression.
        TQRegExp regExp(pattern());

        if (!regExp.isValid())
        {
            KMessageBox::error(this, i18n("Invalid regular expression."));
            return;
        }
    }
    m_find->addToHistory(pattern());
    emit okClicked();
    if ( testWFlags( WShowModal ) )
        accept();
}
#include "kfinddialog.moc"
