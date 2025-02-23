/* This file is part of the KDE libraries
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

   Based on work of:
     Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

//BEGIN Includes
#include "katedialogs.h"
#include "katedialogs.moc"

#include "kateautoindent.h"
#include "katebuffer.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "katefactory.h"
#include "kateschema.h"
#include "katesyntaxdocument.h"
#include "kateview.h"


#include <tdetexteditor/configinterfaceextension.h>
#include <tdetexteditor/plugin.h>

#include <tdeio/job.h>
#include <tdeio/jobclasses.h>
#include <tdeio/netaccess.h>

#include <tdeaccel.h>
#include <tdeapplication.h>
#include <kbuttonbox.h>
#include <kcharsets.h>
#include <kcolorbutton.h>
#include <kcolorcombo.h>
#include <kcolordialog.h>
#include <kcombobox.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdefontdialog.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <kkeybutton.h>
#include <kkeydialog.h>
#include <klineedit.h>
#include <tdelistview.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kmimetypechooser.h>
#include <knuminput.h>
#include <tdeparts/componentfactory.h>
#include <tdepopupmenu.h>
#include <kprocess.h>
#include <kprocio.h>
#include <kregexpeditorinterface.h>
#include <krun.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <tdetempfile.h>

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdialog.h>
#include <tqdom.h>
#include <tqfile.h>
#include <tqgrid.h>
#include <tqgroupbox.h>
#include <tqhbox.h>
#include <tqheader.h>
#include <tqhgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlistbox.h>
#include <tqlistview.h>
#include <tqmap.h>
#include <tqobjectlist.h>
#include <tqpainter.h>
#include <tqpointarray.h>
#include <tqptrcollection.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqslider.h>
#include <tqspinbox.h>
#include <tqstringlist.h>
#include <tqtabwidget.h>
#include <tqtextcodec.h>
#include <tqtoolbutton.h>
#include <tqvbox.h>
#include <tqvgroupbox.h>
#include <tqwhatsthis.h>
#include <tqwidgetstack.h>

#define HLDOWNLOADPATH "http://git.trinitydesktop.org/cgit/tdelibs/plain/kate/data/update-files.xml"

//END

//BEGIN KateConfigPage
KateConfigPage::KateConfigPage ( TQWidget *parent, const char *name )
  : Kate::ConfigPage (parent, name)
  , m_changed (false)
{
  connect (this, TQ_SIGNAL(changed()), this, TQ_SLOT(somethingHasChanged ()));
}

KateConfigPage::~KateConfigPage ()
{
}

void KateConfigPage::somethingHasChanged ()
{
  m_changed = true;
  kdDebug (13000) << "TEST: something changed on the config page: " << this << endl;
}
//END KateConfigPage

//BEGIN KateIndentConfigTab
const int KateIndentConfigTab::flags[] = {
    KateDocument::cfSpaceIndent,
    KateDocument::cfKeepIndentProfile,
    KateDocument::cfKeepExtraSpaces,
    KateDocument::cfTabIndents,
    KateDocument::cfBackspaceIndents,
    KateDocumentConfig::cfDoxygenAutoTyping,
    KateDocumentConfig::cfMixedIndent,
    KateDocumentConfig::cfIndentPastedText
};

KateIndentConfigTab::KateIndentConfigTab(TQWidget *parent)
  : KateConfigPage(parent)
{
  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );
  int configFlags = KateDocumentConfig::global()->configFlags();

  TQVGroupBox *gbAuto = new TQVGroupBox(i18n("Automatic Indentation"), this);

  TQHBox *indentLayout = new TQHBox(gbAuto);
  indentLayout->setSpacing(KDialog::spacingHint());
  TQLabel *indentLabel = new TQLabel(i18n("&Indentation mode:"), indentLayout);
  m_indentMode = new KComboBox (indentLayout);
  m_indentMode->insertStringList (KateAutoIndent::listModes());
  indentLabel->setBuddy(m_indentMode);
  m_configPage = new TQPushButton(SmallIconSet("configure"), i18n("Configure..."), indentLayout);

  opt[5] = new TQCheckBox(i18n("Insert leading Doxygen \"*\" when typing"), gbAuto);
  opt[7] = new TQCheckBox(i18n("Adjust indentation of code pasted from the clipboard"), gbAuto);

  TQVGroupBox *gbSpaces = new TQVGroupBox(i18n("Indentation with Spaces"), this);
  TQVBox *spaceLayout = new TQVBox(gbSpaces);
  opt[0] = new TQCheckBox(i18n("Use &spaces instead of tabs to indent"), spaceLayout );
  opt[6] = new TQCheckBox(i18n("Emacs style mixed mode"), spaceLayout);

  indentationWidth = new KIntNumInput(KateDocumentConfig::global()->indentationWidth(), spaceLayout);
  indentationWidth->setRange(1, 16, 1, false);
  indentationWidth->setLabel(i18n("Number of spaces:"), AlignVCenter);

  opt[1] = new TQCheckBox(i18n("Keep indent &profile"), this);
  opt[2] = new TQCheckBox(i18n("&Keep extra spaces"), this);

  TQVGroupBox *keys = new TQVGroupBox(i18n("Keys to Use"), this);
  opt[3] = new TQCheckBox(i18n("&Tab key indents"), keys);
  opt[4] = new TQCheckBox(i18n("&Backspace key indents"), keys);

  TQRadioButton *rb1, *rb2, *rb3;
  m_tabs = new TQButtonGroup( 1, TQt::Horizontal, i18n("Tab Key Mode if Nothing Selected"), this );
  m_tabs->setRadioButtonExclusive( true );
  m_tabs->insert( rb1=new TQRadioButton( i18n("Insert indent &characters"), m_tabs ), 0 );
  m_tabs->insert( rb2=new TQRadioButton( i18n("I&nsert tab character"), m_tabs ), 1 );
  m_tabs->insert( rb3=new TQRadioButton( i18n("Indent current &line"), m_tabs ), 2 );

  opt[0]->setChecked(configFlags & flags[0]);
  opt[1]->setChecked(configFlags & flags[1]);
  opt[2]->setChecked(configFlags & flags[2]);
  opt[3]->setChecked(configFlags & flags[3]);
  opt[4]->setChecked(configFlags & flags[4]);
  opt[5]->setChecked(configFlags & flags[5]);
  opt[6]->setChecked(configFlags & flags[6]);
  opt[7]->setChecked(configFlags & flags[7]);

  layout->addWidget(gbAuto);
  layout->addWidget(gbSpaces);
  layout->addWidget(opt[1]);
  layout->addWidget(opt[2]);
  layout->addWidget(keys);
  layout->addWidget(m_tabs, 0);

  layout->addStretch();

  // What is this? help
  TQWhatsThis::add(opt[0], i18n(
        "Check this if you want to indent with spaces rather than tabs."));
  TQWhatsThis::add(opt[2], i18n(
        "Indentations of more than the selected number of spaces will not be "
        "shortened."));
  TQWhatsThis::add(opt[3], i18n(
        "This allows the <b>Tab</b> key to be used to increase the indentation "
        "level."));
  TQWhatsThis::add(opt[4], i18n(
        "This allows the <b>Backspace</b> key to be used to decrease the "
        "indentation level."));
  TQWhatsThis::add(opt[5], i18n(
        "Automatically inserts a leading \"*\" while typing within a Doxygen "
        "style comment."));
  TQWhatsThis::add( opt[6], i18n(
      "Use a mix of tab and space characters for indentation.") );
  TQWhatsThis::add( opt[7], i18n(
      "If this option is selected, pasted code from the clipboard is indented. "
      "Triggering the <b>undo</b>-action removes the indentation.") );
  TQWhatsThis::add(indentationWidth, i18n("The number of spaces to indent with."));

  TQWhatsThis::add(m_configPage, i18n(
        "If this button is enabled, additional indenter specific options are "
        "available and can be configured in an extra dialog.") );

  reload ();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(m_indentMode, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotChanged()));
  connect(m_indentMode, TQ_SIGNAL(activated(int)), this, TQ_SLOT(indenterSelected(int)));

  connect( opt[0], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(somethingToggled()));

  connect( opt[0], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[1], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[2], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[3], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[4], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[5], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[6], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( opt[7], TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );

  connect(indentationWidth, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));

  connect(rb1, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(rb2, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(rb3, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  connect(m_configPage, TQ_SIGNAL(clicked()), this, TQ_SLOT(configPage()));
}

void KateIndentConfigTab::somethingToggled() {
  indentationWidth->setEnabled(opt[0]->isChecked());
  opt[6]->setEnabled(opt[0]->isChecked());
}

void KateIndentConfigTab::indenterSelected (int index)
{
  if (index == KateDocumentConfig::imCStyle || index == KateDocumentConfig::imCSAndS)
    opt[5]->setEnabled(true);
  else
    opt[5]->setEnabled(false);

  m_configPage->setEnabled( KateAutoIndent::hasConfigPage(index) );
}

void KateIndentConfigTab::configPage()
{
  uint index = m_indentMode->currentItem();
  if ( KateAutoIndent::hasConfigPage(index) )
  {
    KDialogBase dlg(this, "indenter_config_dialog", true, i18n("Configure Indenter"),
      KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Cancel, true);

    TQVBox *box = new TQVBox(&dlg);
    box->setSpacing( KDialog::spacingHint() );
    dlg.setMainWidget(box);
    new TQLabel("<qt><b>" + KateAutoIndent::modeDescription(index) + "</b></qt>", box);
    new KSeparator(KSeparator::HLine, box);

    IndenterConfigPage* page = KateAutoIndent::configPage(box, index);

    if (!page) return;
    box->setStretchFactor(page, 1);

    connect( &dlg, TQ_SIGNAL(okClicked()), page, TQ_SLOT(apply()) );

    dlg.resize(400, 300);
    dlg.exec();
  }
}

void KateIndentConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  KateDocumentConfig::global()->configStart ();

  int configFlags, z;

  configFlags = KateDocumentConfig::global()->configFlags();
  for (z = 0; z < numFlags; z++) {
    configFlags &= ~flags[z];
    if (opt[z]->isChecked()) configFlags |= flags[z];
  }

  KateDocumentConfig::global()->setConfigFlags(configFlags);
  KateDocumentConfig::global()->setIndentationWidth(indentationWidth->value());

  KateDocumentConfig::global()->setIndentationMode(m_indentMode->currentItem());

  KateDocumentConfig::global()->setConfigFlags (KateDocumentConfig::cfTabIndentsMode, 2 == m_tabs->id (m_tabs->selected()));
  KateDocumentConfig::global()->setConfigFlags (KateDocumentConfig::cfTabInsertsTab, 1 == m_tabs->id (m_tabs->selected()));

  KateDocumentConfig::global()->configEnd ();
}

void KateIndentConfigTab::reload ()
{
  if (KateDocumentConfig::global()->configFlags() & KateDocumentConfig::cfTabIndentsMode)
    m_tabs->setButton (2);
  else if (KateDocumentConfig::global()->configFlags() & KateDocumentConfig::cfTabInsertsTab)
    m_tabs->setButton (1);
  else
    m_tabs->setButton (0);

  m_indentMode->setCurrentItem (KateDocumentConfig::global()->indentationMode());

  somethingToggled ();
  indenterSelected (m_indentMode->currentItem());
}
//END KateIndentConfigTab

//BEGIN KateSelectConfigTab
const int KateSelectConfigTab::flags[] = {};

KateSelectConfigTab::KateSelectConfigTab(TQWidget *parent)
  : KateConfigPage(parent)
{
  int configFlags = KateDocumentConfig::global()->configFlags();

  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );

  TQVGroupBox *gbCursor = new TQVGroupBox(i18n("Text Cursor Movement"), this);

  opt[0] = new TQCheckBox(i18n("Smart ho&me and smart end"), gbCursor);
  opt[0]->setChecked(configFlags & KateDocumentConfig::cfSmartHome);
  connect(opt[0], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  opt[1] = new TQCheckBox(i18n("Wrap c&ursor"), gbCursor);
  opt[1]->setChecked(configFlags & KateDocumentConfig::cfWrapCursor);
  connect(opt[1], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  e6 = new TQCheckBox(i18n("&PageUp/PageDown moves cursor"), gbCursor);
  e6->setChecked(KateDocumentConfig::global()->pageUpDownMovesCursor());
  connect(e6, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  e4 = new KIntNumInput(KateViewConfig::global()->autoCenterLines(), gbCursor);
  e4->setRange(0, 1000000, 1, false);
  e4->setLabel(i18n("Autocenter cursor (lines):"), AlignVCenter);
  connect(e4, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));

  layout->addWidget(gbCursor);

  TQRadioButton *rb1, *rb2;

  m_tabs = new TQButtonGroup( 1, TQt::Horizontal, i18n("Selection Mode"), this );
  layout->add (m_tabs);

  m_tabs->setRadioButtonExclusive( true );
  m_tabs->insert( rb1=new TQRadioButton( i18n("&Normal"), m_tabs ), 0 );
  m_tabs->insert( rb2=new TQRadioButton( i18n("&Persistent"), m_tabs ), 1 );

  layout->addStretch();

  TQWhatsThis::add(rb1, i18n(
        "Selections will be overwritten by typed text and will be lost on "
        "cursor movement."));
  TQWhatsThis::add(rb2, i18n(
        "Selections will stay even after cursor movement and typing."));

  TQWhatsThis::add(e4, i18n(
        "Sets the number of lines to maintain visible above and below the "
        "cursor when possible."));

  TQWhatsThis::add(opt[0], i18n(
        "When selected, pressing the home key will cause the cursor to skip "
        "whitespace and go to the start of a line's text. "
        "The same applies for the end key."));

    TQWhatsThis::add(opt[1], i18n(
        "When on, moving the insertion cursor using the <b>Left</b> and "
        "<b>Right</b> keys will go on to previous/next line at beginning/end of "
        "the line, similar to most editors.<p>When off, the insertion cursor "
        "cannot be moved left of the line start, but it can be moved off the "
        "line end, which can be very handy for programmers."));

  TQWhatsThis::add(e6, i18n("Selects whether the PageUp and PageDown keys should alter the vertical position of the cursor relative to the top of the view."));


  reload ();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(rb1, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(rb2, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
}

void KateSelectConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateDocumentConfig::global()->configStart ();

  int configFlags = KateDocumentConfig::global()->configFlags();

  configFlags &= ~KateDocumentConfig::cfSmartHome;
  configFlags &= ~KateDocumentConfig::cfWrapCursor;

  if (opt[0]->isChecked()) configFlags |= KateDocumentConfig::cfSmartHome;
  if (opt[1]->isChecked()) configFlags |= KateDocumentConfig::cfWrapCursor;

  KateDocumentConfig::global()->setConfigFlags(configFlags);

  KateViewConfig::global()->setAutoCenterLines(kMax(0, e4->value()));
  KateDocumentConfig::global()->setPageUpDownMovesCursor(e6->isChecked());

  KateViewConfig::global()->setPersistentSelection (m_tabs->id (m_tabs->selected()) == 1);

  KateDocumentConfig::global()->configEnd ();
  KateViewConfig::global()->configEnd ();
}

void KateSelectConfigTab::reload ()
{
  if (KateViewConfig::global()->persistentSelection())
    m_tabs->setButton (1);
  else
    m_tabs->setButton (0);
}
//END KateSelectConfigTab

//BEGIN KateEditConfigTab
const int KateEditConfigTab::flags[] = {KateDocument::cfWordWrap,
  KateDocument::cfAutoBrackets, KateDocument::cfShowTabs,
  KateDocumentConfig::cfReplaceTabsDyn, KateDocumentConfig::cfRemoveTrailingDyn};

KateEditConfigTab::KateEditConfigTab(TQWidget *parent)
  : KateConfigPage(parent)
{
  TQVBoxLayout *mainLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );
  int configFlags = KateDocumentConfig::global()->configFlags();

  TQVGroupBox *gbWhiteSpace = new TQVGroupBox(i18n("Tabulators"), this);

  opt[3] = new TQCheckBox( i18n("&Insert spaces instead of tabulators"), gbWhiteSpace );
  opt[3]->setChecked( configFlags & KateDocumentConfig::cfReplaceTabsDyn );
  connect( opt[3], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()) );

  opt[2] = new TQCheckBox(i18n("&Show tabulators"), gbWhiteSpace);
  opt[2]->setChecked(configFlags & flags[2]);
  connect(opt[2], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  e2 = new KIntNumInput(KateDocumentConfig::global()->tabWidth(), gbWhiteSpace);
  e2->setRange(1, 16, 1, false);
  e2->setLabel(i18n("Tab width:"), AlignVCenter);
  connect(e2, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));

  mainLayout->addWidget(gbWhiteSpace);

  TQVGroupBox *gbWordWrap = new TQVGroupBox(i18n("Static Word Wrap"), this);

  opt[0] = new TQCheckBox(i18n("Enable static &word wrap"), gbWordWrap);
  opt[0]->setChecked(KateDocumentConfig::global()->wordWrap());
  connect(opt[0], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  m_wwmarker = new TQCheckBox( i18n("&Show static word wrap marker (if applicable)"), gbWordWrap );
  m_wwmarker->setChecked( KateRendererConfig::global()->wordWrapMarker() );
  connect(m_wwmarker, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  e1 = new KIntNumInput(KateDocumentConfig::global()->wordWrapAt(), gbWordWrap);
  e1->setRange(20, 200, 1, false);
  e1->setLabel(i18n("Wrap words at:"), AlignVCenter);
  connect(e1, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));

  mainLayout->addWidget(gbWordWrap);

  opt[4] = new TQCheckBox( i18n("Remove &trailing spaces"), this );
  mainLayout->addWidget( opt[4] );
  opt[4]->setChecked( configFlags & KateDocumentConfig::cfRemoveTrailingDyn );
  connect( opt[4], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()) );

  opt[1] = new TQCheckBox(i18n("Auto &brackets"), this);
  mainLayout->addWidget(opt[1]);
  opt[1]->setChecked(configFlags & flags[1]);
  connect(opt[1], TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));

  e3 = new KIntNumInput(e2, KateDocumentConfig::global()->undoSteps(), this);
  e3->setRange(0, 1000000, 1, false);
  e3->setSpecialValueText( i18n("Unlimited") );
  e3->setLabel(i18n("Maximum undo steps:"), AlignVCenter);
  mainLayout->addWidget(e3);
  connect(e3, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));

  TQHBoxLayout *e5Layout = new TQHBoxLayout(mainLayout);
  TQLabel *e5Label = new TQLabel(i18n("Smart search t&ext from:"), this);
  e5Layout->addWidget(e5Label);
  e5 = new KComboBox (this);
  e5->insertItem( i18n("Nowhere") );
  e5->insertItem( i18n("Selection Only") );
  e5->insertItem( i18n("Selection, then Current Word") );
  e5->insertItem( i18n("Current Word Only") );
  e5->insertItem( i18n("Current Word, then Selection") );
  e5->setCurrentItem(KateViewConfig::global()->textToSearchMode());
  e5Layout->addWidget(e5);
  e5Label->setBuddy(e5);
  connect(e5, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotChanged()));

  mainLayout->addStretch();

  // What is this? help
  TQWhatsThis::add(opt[0], i18n(
        "Automatically start a new line of text when the current line exceeds "
        "the length specified by the <b>Wrap words at:</b> option."
        "<p>This option does not wrap existing lines of text - use the <b>Apply "
        "Static Word Wrap</b> option in the <b>Tools</b> menu for that purpose."
        "<p>If you want lines to be <i>visually wrapped</i> instead, according "
        "to the width of the view, enable <b>Dynamic Word Wrap</b> in the "
        "<b>View Defaults</b> config page."));
  TQWhatsThis::add(e1, i18n(
        "If the Word Wrap option is selected this entry determines the length "
        "(in characters) at which the editor will automatically start a new line."));
  TQWhatsThis::add(opt[1], i18n(
        "When the user types a left bracket ([,(, or {) KateView automatically "
        "enters the right bracket (}, ), or ]) to the right of the cursor."));
  TQWhatsThis::add(opt[2], i18n(
        "The editor will display a symbol to indicate the presence of a tab in "
        "the text."));

  TQWhatsThis::add(e3, i18n(
        "Sets the number of undo/redo steps to record. More steps uses more memory."));

  TQString gstfwt = i18n(
        "This determines where KateView will get the search text from "
        "(this will be automatically entered into the Find Text dialog): "
        "<br>"
        "<ul>"
        "<li><b>Nowhere:</b> Don't guess the search text."
        "</li>"
        "<li><b>Selection Only:</b> Use the current text selection, "
        "if available."
        "</li>"
        "<li><b>Selection, then Current Word:</b> Use the current "
        "selection if available, otherwise use the current word."
        "</li>"
        "<li><b>Current Word Only:</b> Use the word that the cursor "
        "is currently resting on, if available."
        "</li>"
        "<li><b>Current Word, then Selection:</b> Use the current "
        "word if available, otherwise use the current selection."
        "</li>"
        "</ul>"
        "Note that, in all the above modes, if a search string has "
        "not been or cannot be determined, then the Find Text Dialog "
        "will fall back to the last search text.");
  TQWhatsThis::add(e5Label, gstfwt);
  TQWhatsThis::add(e5, gstfwt);
  TQWhatsThis::add( opt[3], i18n(
      "If this is enabled, the editor will calculate the number of spaces up to "
      "the next tab position as defined by the tab width, and insert that number "
      "of spaces instead of a TAB character." ) );
  TQWhatsThis::add( opt[4], i18n(
      "If this is enabled, the editor will remove any trailing whitespace on "
      "lines when they are left by the insertion cursor.") );
  TQWhatsThis::add( m_wwmarker, i18n(
        "<p>If this option is checked, a vertical line will be drawn at the word "
        "wrap column as defined in the <strong>Editing</strong> properties."
        "<p>Note that the word wrap marker is only drawn if you use a fixed "
        "pitch font." ));
}

void KateEditConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateDocumentConfig::global()->configStart ();

  int configFlags, z;

  configFlags = KateDocumentConfig::global()->configFlags();
  for (z = 1; z < numFlags; z++) {
    configFlags &= ~flags[z];
    if (opt[z]->isChecked()) configFlags |= flags[z];
  }
  KateDocumentConfig::global()->setConfigFlags(configFlags);

  KateDocumentConfig::global()->setWordWrapAt(e1->value());
  KateDocumentConfig::global()->setWordWrap (opt[0]->isChecked());
  KateDocumentConfig::global()->setTabWidth(e2->value());

  if (e3->value() <= 0)
    KateDocumentConfig::global()->setUndoSteps(0);
  else
    KateDocumentConfig::global()->setUndoSteps(e3->value());

  KateViewConfig::global()->setTextToSearchMode(e5->currentItem());

  KateRendererConfig::global()->setWordWrapMarker (m_wwmarker->isChecked());

  KateDocumentConfig::global()->configEnd ();
  KateViewConfig::global()->configEnd ();
}

void KateEditConfigTab::reload ()
{
}
//END KateEditConfigTab

//BEGIN KateViewDefaultsConfig
KateViewDefaultsConfig::KateViewDefaultsConfig(TQWidget *parent)
  :KateConfigPage(parent)
{
  TQRadioButton *rb1;
  TQRadioButton *rb2;

  TQVBoxLayout *blay=new TQVBoxLayout(this,0,KDialog::spacingHint());

  TQVGroupBox *gbWordWrap = new TQVGroupBox(i18n("Word Wrap"), this);

  m_dynwrap=new TQCheckBox(i18n("&Dynamic word wrap"),gbWordWrap);

  TQHBox *m_dynwrapIndicatorsLay = new TQHBox (gbWordWrap);
  m_dynwrapIndicatorsLabel = new TQLabel( i18n("Dynamic word wrap indicators (if applicable):"), m_dynwrapIndicatorsLay );
  m_dynwrapIndicatorsCombo = new KComboBox( m_dynwrapIndicatorsLay );
  m_dynwrapIndicatorsCombo->insertItem( i18n("Off") );
  m_dynwrapIndicatorsCombo->insertItem( i18n("Follow Line Numbers") );
  m_dynwrapIndicatorsCombo->insertItem( i18n("Always On") );
  m_dynwrapIndicatorsLabel->setBuddy(m_dynwrapIndicatorsCombo);

  m_dynwrapAlignLevel = new KIntNumInput(gbWordWrap);
  m_dynwrapAlignLevel->setLabel(i18n("Vertically align dynamically wrapped lines to indentation depth:"));
  m_dynwrapAlignLevel->setRange(0, 80, 10);
  // xgettext:no-c-format
  m_dynwrapAlignLevel->setSuffix(i18n("% of View Width"));
  m_dynwrapAlignLevel->setSpecialValueText(i18n("Disabled"));

  blay->addWidget(gbWordWrap);

  TQVGroupBox *gbFold = new TQVGroupBox(i18n("Code Folding"), this);

  m_folding=new TQCheckBox(i18n("Show &folding markers (if available)"), gbFold );
  m_collapseTopLevel = new TQCheckBox( i18n("Collapse toplevel folding nodes"), gbFold );
  m_collapseTopLevel->hide ();

  blay->addWidget(gbFold);

  TQVGroupBox *gbBar = new TQVGroupBox(i18n("Borders"), this);

  m_icons=new TQCheckBox(i18n("Show &icon border"),gbBar);
  m_line=new TQCheckBox(i18n("Show &line numbers"),gbBar);
  m_scrollBarMarks=new TQCheckBox(i18n("Show &scrollbar marks"),gbBar);

  blay->addWidget(gbBar);

  m_bmSort = new TQButtonGroup( 1, TQt::Horizontal, i18n("Sort Bookmarks Menu"), this );
  m_bmSort->setRadioButtonExclusive( true );
  m_bmSort->insert( rb1=new TQRadioButton( i18n("By &position"), m_bmSort ), 0 );
  m_bmSort->insert( rb2=new TQRadioButton( i18n("By c&reation"), m_bmSort ), 1 );

  blay->addWidget(m_bmSort, 0 );

  m_showIndentLines = new TQCheckBox(i18n("Show indentation lines"), this);
  m_showIndentLines->setChecked(KateRendererConfig::global()->showIndentationLines());
  blay->addWidget(m_showIndentLines);

  blay->addStretch(1000);

  TQWhatsThis::add(m_dynwrap,i18n(
        "If this option is checked, the text lines will be wrapped at the view "
        "border on the screen."));
  TQString wtstr = i18n("Choose when the Dynamic Word Wrap Indicators should be displayed");
  TQWhatsThis::add(m_dynwrapIndicatorsLabel, wtstr);
  TQWhatsThis::add(m_dynwrapIndicatorsCombo, wtstr);
  // xgettext:no-c-format
  TQWhatsThis::add(m_dynwrapAlignLevel, i18n(
        "<p>Enables the start of dynamically wrapped lines to be aligned "
        "vertically to the indentation level of the first line.  This can help "
        "to make code and markup more readable.</p><p>Additionally, this allows "
        "you to set a maximum width of the screen, as a percentage, after which "
        "dynamically wrapped lines will no longer be vertically aligned.  For "
        "example, at 50%, lines whose indentation levels are deeper than 50% of "
        "the width of the screen will not have vertical alignment applied to "
        "subsequent wrapped lines.</p>"));
  TQWhatsThis::add(m_line,i18n(
        "If this option is checked, every new view will display line numbers "
        "on the left hand side."));
  TQWhatsThis::add(m_icons,i18n(
        "If this option is checked, every new view will display an icon border "
        "on the left hand side.<br><br>The icon border shows bookmark signs, "
        "for instance."));
  TQWhatsThis::add(m_scrollBarMarks,i18n(
        "If this option is checked, every new view will show marks on the "
        "vertical scrollbar.<br><br>These marks will, for instance, show "
        "bookmarks."));
  TQWhatsThis::add(m_folding,i18n(
        "If this option is checked, every new view will display marks for code "
        "folding, if code folding is available."));
  TQWhatsThis::add(m_bmSort,i18n(
        "Choose how the bookmarks should be ordered in the <b>Bookmarks</b> menu."));
  TQWhatsThis::add(rb1,i18n(
        "The bookmarks will be ordered by the line numbers they are placed at."));
  TQWhatsThis::add(rb2,i18n(
        "Each new bookmark will be added to the bottom, independently from "
        "where it is placed in the document."));
  TQWhatsThis::add(m_showIndentLines, i18n(
        "If this is enabled, the editor will display vertical lines to help "
        "identify indent lines.") );

  reload();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(m_dynwrap, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(m_dynwrapIndicatorsCombo, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotChanged()));
  connect(m_dynwrapAlignLevel, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));
  connect(m_icons, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(m_scrollBarMarks, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(m_line, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(m_folding, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(m_collapseTopLevel, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()) );
  connect(rb1, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(rb2, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect(m_showIndentLines, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
}

KateViewDefaultsConfig::~KateViewDefaultsConfig()
{
}

void KateViewDefaultsConfig::apply ()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateRendererConfig::global()->configStart ();

  KateViewConfig::global()->setDynWordWrap (m_dynwrap->isChecked());
  KateViewConfig::global()->setDynWordWrapIndicators (m_dynwrapIndicatorsCombo->currentItem ());
  KateViewConfig::global()->setDynWordWrapAlignIndent(m_dynwrapAlignLevel->value());
  KateViewConfig::global()->setLineNumbers (m_line->isChecked());
  KateViewConfig::global()->setIconBar (m_icons->isChecked());
  KateViewConfig::global()->setScrollBarMarks (m_scrollBarMarks->isChecked());
  KateViewConfig::global()->setFoldingBar (m_folding->isChecked());
  KateViewConfig::global()->setBookmarkSort (m_bmSort->id (m_bmSort->selected()));

  KateRendererConfig::global()->setShowIndentationLines(m_showIndentLines->isChecked());

  KateRendererConfig::global()->configEnd ();
  KateViewConfig::global()->configEnd ();
}

void KateViewDefaultsConfig::reload ()
{
  m_dynwrap->setChecked(KateViewConfig::global()->dynWordWrap());
  m_dynwrapIndicatorsCombo->setCurrentItem( KateViewConfig::global()->dynWordWrapIndicators() );
  m_dynwrapAlignLevel->setValue(KateViewConfig::global()->dynWordWrapAlignIndent());
  m_line->setChecked(KateViewConfig::global()->lineNumbers());
  m_icons->setChecked(KateViewConfig::global()->iconBar());
  m_scrollBarMarks->setChecked(KateViewConfig::global()->scrollBarMarks());
  m_folding->setChecked(KateViewConfig::global()->foldingBar());
  m_bmSort->setButton( KateViewConfig::global()->bookmarkSort() );
  m_showIndentLines->setChecked(KateRendererConfig::global()->showIndentationLines());
}

void KateViewDefaultsConfig::reset () {;}

void KateViewDefaultsConfig::defaults (){;}
//END KateViewDefaultsConfig

//BEGIN KateEditKeyConfiguration

KateEditKeyConfiguration::KateEditKeyConfiguration( TQWidget* parent, KateDocument* doc )
  : KateConfigPage( parent )
{
  m_doc = doc;
  m_ready = false;
}

void KateEditKeyConfiguration::showEvent ( TQShowEvent * )
{
  if (!m_ready)
  {
    (new TQVBoxLayout(this))->setAutoAdd(true);
    KateView* view = (KateView*)m_doc->views().at(0);
    m_ac = view->editActionCollection();
    m_keyChooser = new KKeyChooser( m_ac, this, false );
    connect( m_keyChooser, TQ_SIGNAL( keyChange() ), this, TQ_SLOT( slotChanged() ) );
    m_keyChooser->show ();

    m_ready = true;
  }

  TQWidget::show ();
}

void KateEditKeyConfiguration::apply()
{
  if ( ! changed() )
    return;
  m_changed = false;

  if (m_ready)
  {
    m_keyChooser->commitChanges();
    m_ac->writeShortcutSettings( "Katepart Shortcuts" );
  }
}
//END KateEditKeyConfiguration

//BEGIN KateSaveConfigTab
KateSaveConfigTab::KateSaveConfigTab( TQWidget *parent )
  : KateConfigPage( parent )
{
  int configFlags = KateDocumentConfig::global()->configFlags();
  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );

  TQVGroupBox *gbEnc = new TQVGroupBox(i18n("File Format"), this);
  layout->addWidget( gbEnc );

  TQHBox *e5Layout = new TQHBox(gbEnc);
  TQLabel *e5Label = new TQLabel(i18n("&Encoding:"), e5Layout);
  m_encoding = new KComboBox (e5Layout);
  e5Label->setBuddy(m_encoding);

  e5Layout = new TQHBox(gbEnc);
  e5Label = new TQLabel(i18n("End &of line:"), e5Layout);
  m_eol = new KComboBox (e5Layout);
  e5Label->setBuddy(m_eol);

  allowEolDetection = new TQCheckBox(i18n("&Automatic end of line detection"), gbEnc);

  m_eol->insertItem (i18n("UNIX"));
  m_eol->insertItem (i18n("DOS/Windows"));
  m_eol->insertItem (i18n("Macintosh"));

  TQVGroupBox *gbMem = new TQVGroupBox(i18n("Memory Usage"), this);
  layout->addWidget( gbMem );

  e5Layout = new TQHBox(gbMem);
  e5Layout->setSpacing (32);
  blockCountLabel = new TQLabel(i18n("Maximum loaded &blocks per file:"), e5Layout);
  blockCount = new TQSpinBox (4, 512, 4, e5Layout);

  blockCount->setValue (KateBuffer::maxLoadedBlocks());
  blockCountLabel->setBuddy(blockCount);

  TQVGroupBox *gbWhiteSpace = new TQVGroupBox(i18n("Automatic Cleanups on Load/Save"), this);
  layout->addWidget( gbWhiteSpace );

  removeSpaces = new TQCheckBox(i18n("Re&move trailing spaces"), gbWhiteSpace);
  removeSpaces->setChecked(configFlags & KateDocument::cfRemoveSpaces);

  TQVGroupBox *dirConfigBox = new TQVGroupBox(i18n("Folder Config File"), this);
  layout->addWidget( dirConfigBox );

  dirSearchDepth = new KIntNumInput(KateDocumentConfig::global()->searchDirConfigDepth(), dirConfigBox);
  dirSearchDepth->setRange(-1, 64, 1, false);
  dirSearchDepth->setSpecialValueText( i18n("Do not use config file") );
  dirSearchDepth->setLabel(i18n("Se&arch depth for config file:"), AlignVCenter);

  TQGroupBox *gb = new TQGroupBox( 1, TQt::Horizontal, i18n("Backup on Save"), this );
  layout->addWidget( gb );
  cbLocalFiles = new TQCheckBox( i18n("&Local files"), gb );
  cbRemoteFiles = new TQCheckBox( i18n("&Remote files"), gb );

  TQHBox *hbBuPrefix = new TQHBox( gb );
  TQLabel *lBuPrefix = new TQLabel( i18n("&Prefix:"), hbBuPrefix );
  leBuPrefix = new TQLineEdit( hbBuPrefix );
  lBuPrefix->setBuddy( leBuPrefix );

  TQHBox *hbBuSuffix = new TQHBox( gb );
  TQLabel *lBuSuffix = new TQLabel( i18n("&Suffix:"), hbBuSuffix );
  leBuSuffix = new TQLineEdit( hbBuSuffix );
  lBuSuffix->setBuddy( leBuSuffix );

  layout->addStretch();

  TQWhatsThis::add(removeSpaces, i18n(
        "The editor will automatically eliminate extra spaces at the ends of "
        "lines of text while loading/saving the file."));
  TQWhatsThis::add( gb, i18n(
        "<p>Backing up on save will cause Kate to copy the disk file to "
        "'&lt;prefix&gt;&lt;filename&gt;&lt;suffix&gt;' before saving changes."
        "<p>The suffix defaults to <strong>~</strong> and prefix is empty by default" ) );
  TQWhatsThis::add( allowEolDetection, i18n(
        "Check this if you want the editor to autodetect the end of line type."
        "The first found end of line type will be used for the whole file.") );
  TQWhatsThis::add( cbLocalFiles, i18n(
        "Check this if you want backups of local files when saving") );
  TQWhatsThis::add( cbRemoteFiles, i18n(
        "Check this if you want backups of remote files when saving") );
  TQWhatsThis::add( leBuPrefix, i18n(
        "Enter the prefix to prepend to the backup file names" ) );
  TQWhatsThis::add( leBuSuffix, i18n(
        "Enter the suffix to add to the backup file names" ) );
  TQWhatsThis::add(dirSearchDepth, i18n(
        "The editor will search the given number of folder levels upwards for .kateconfig file"
        " and load the settings line from it." ));
  TQWhatsThis::add(blockCount, i18n(
        "The editor will load given number of blocks (of around 2048 lines) of text into memory;"
        " if the filesize is bigger than this the other blocks are swapped "
        " to disk and loaded transparently as-needed.<br>"
        " This can cause little delays while navigating in the document; a larger block count"
        " increases the editing speed at the cost of memory. <br>For normal usage, just choose the highest possible"
        " block count: limit it only if you have problems with the memory usage."));

  reload();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(m_encoding, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotChanged()));
  connect(m_eol, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotChanged()));
  connect( allowEolDetection, TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect(blockCount, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));
  connect(removeSpaces, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(slotChanged()));
  connect( cbLocalFiles, TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect( cbRemoteFiles, TQ_SIGNAL( toggled(bool) ), this, TQ_SLOT( slotChanged() ) );
  connect(dirSearchDepth, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(slotChanged()));
  connect( leBuPrefix, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( leBuSuffix, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
}

void KateSaveConfigTab::apply()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  KateBuffer::setMaxLoadedBlocks (blockCount->value());

  KateDocumentConfig::global()->configStart ();

  if ( leBuSuffix->text().isEmpty() && leBuPrefix->text().isEmpty() ) {
    KMessageBox::information(
                this,
                i18n("You did not provide a backup suffix or prefix. Using default suffix: '~'"),
                i18n("No Backup Suffix or Prefix")
                        );
    leBuSuffix->setText( "~" );
  }

  uint f( 0 );
  if ( cbLocalFiles->isChecked() )
    f |= KateDocumentConfig::LocalFiles;
  if ( cbRemoteFiles->isChecked() )
    f |= KateDocumentConfig::RemoteFiles;

  KateDocumentConfig::global()->setBackupFlags(f);
  KateDocumentConfig::global()->setBackupPrefix(leBuPrefix->text());
  KateDocumentConfig::global()->setBackupSuffix(leBuSuffix->text());

  KateDocumentConfig::global()->setSearchDirConfigDepth(dirSearchDepth->value());

  int configFlags = KateDocumentConfig::global()->configFlags();

  configFlags &= ~KateDocument::cfRemoveSpaces; // clear flag
  if (removeSpaces->isChecked()) configFlags |= KateDocument::cfRemoveSpaces; // set flag if checked

  KateDocumentConfig::global()->setConfigFlags(configFlags);

  KateDocumentConfig::global()->setEncoding((m_encoding->currentItem() == 0) ? "" : TDEGlobal::charsets()->encodingForName(m_encoding->currentText()));

  KateDocumentConfig::global()->setEol(m_eol->currentItem());
  KateDocumentConfig::global()->setAllowEolDetection(allowEolDetection->isChecked());

  KateDocumentConfig::global()->configEnd ();
}

void KateSaveConfigTab::reload()
{
  // encoding
  m_encoding->clear ();
  m_encoding->insertItem (i18n("TDE Default"));
  m_encoding->setCurrentItem(0);
  TQStringList encodings (TDEGlobal::charsets()->descriptiveEncodingNames());
  int insert = 1;
  for (uint i=0; i < encodings.count(); i++)
  {
    bool found = false;
    TQTextCodec *codecForEnc = TDEGlobal::charsets()->codecForName(TDEGlobal::charsets()->encodingForName(encodings[i]), found);

    if (found)
    {
      m_encoding->insertItem (encodings[i]);

      if ( codecForEnc->name() == KateDocumentConfig::global()->encoding() )
      {
        m_encoding->setCurrentItem(insert);
      }

      insert++;
    }
  }

  // eol
  m_eol->setCurrentItem(KateDocumentConfig::global()->eol());
  allowEolDetection->setChecked(KateDocumentConfig::global()->allowEolDetection());

  dirSearchDepth->setValue(KateDocumentConfig::global()->searchDirConfigDepth());

  // other stuff
  uint f ( KateDocumentConfig::global()->backupFlags() );
  cbLocalFiles->setChecked( f & KateDocumentConfig::LocalFiles );
  cbRemoteFiles->setChecked( f & KateDocumentConfig::RemoteFiles );
  leBuPrefix->setText( KateDocumentConfig::global()->backupPrefix() );
  leBuSuffix->setText( KateDocumentConfig::global()->backupSuffix() );
}

void KateSaveConfigTab::reset()
{
}

void KateSaveConfigTab::defaults()
{
  cbLocalFiles->setChecked( true );
  cbRemoteFiles->setChecked( false );
  leBuPrefix->setText( "" );
  leBuSuffix->setText( "~" );
}

//END KateSaveConfigTab

//BEGIN PluginListItem
class KatePartPluginListItem : public TQCheckListItem
{
  public:
    KatePartPluginListItem(bool checked, uint i, const TQString &name, TQListView *parent);
    uint pluginIndex () const { return index; }

  protected:
    void stateChange(bool);

  private:
    uint index;
    bool silentStateChange;
};

KatePartPluginListItem::KatePartPluginListItem(bool checked, uint i, const TQString &name, TQListView *parent)
  : TQCheckListItem(parent, name, CheckBox)
  , index(i)
  , silentStateChange(false)
{
  silentStateChange = true;
  setOn(checked);
  silentStateChange = false;
}

void KatePartPluginListItem::stateChange(bool b)
{
  if(!silentStateChange)
    static_cast<KatePartPluginListView *>(listView())->stateChanged(this, b);
}
//END

//BEGIN PluginListView
KatePartPluginListView::KatePartPluginListView(TQWidget *parent, const char *name)
  : TDEListView(parent, name)
{
}

void KatePartPluginListView::stateChanged(KatePartPluginListItem *item, bool b)
{
  emit stateChange(item, b);
}
//END

//BEGIN KatePartPluginConfigPage
KatePartPluginConfigPage::KatePartPluginConfigPage (TQWidget *parent) : KateConfigPage (parent, "")
{
  // sizemanagment
  TQGridLayout *grid = new TQGridLayout( this, 1, 1 );
  grid->setSpacing( KDialogBase::spacingHint() );

  listView = new KatePartPluginListView(this);
  listView->addColumn(i18n("Name"));
  listView->addColumn(i18n("Comment"));

  grid->addWidget( listView, 0, 0);

  for (uint i=0; i<KateFactory::self()->plugins().count(); i++)
  {
    KatePartPluginListItem *item = new KatePartPluginListItem(KateDocumentConfig::global()->plugin(i), i, (KateFactory::self()->plugins())[i]->name(), listView);
    item->setText(0, (KateFactory::self()->plugins())[i]->name());
    item->setText(1, (KateFactory::self()->plugins())[i]->comment());

    m_items.append (item);
  }

  // configure button

  btnConfigure = new TQPushButton( i18n("Configure..."), this );
  btnConfigure->setEnabled( false );
  grid->addWidget( btnConfigure, 1, 0, TQt::AlignRight );
  connect( btnConfigure, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotConfigure()) );

  connect( listView, TQ_SIGNAL(selectionChanged(TQListViewItem*)), this, TQ_SLOT(slotCurrentChanged(TQListViewItem*)) );
  connect( listView, TQ_SIGNAL(stateChange(KatePartPluginListItem *, bool)),
    this, TQ_SLOT(slotStateChanged(KatePartPluginListItem *, bool)));
  connect(listView, TQ_SIGNAL(stateChange(KatePartPluginListItem *, bool)), this, TQ_SLOT(slotChanged()));
}

KatePartPluginConfigPage::~KatePartPluginConfigPage ()
{
}

void KatePartPluginConfigPage::apply ()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  KateDocumentConfig::global()->configStart ();

  for (uint i=0; i < m_items.count(); i++)
    KateDocumentConfig::global()->setPlugin (m_items.at(i)->pluginIndex(), m_items.at(i)->isOn());

  KateDocumentConfig::global()->configEnd ();
}

void KatePartPluginConfigPage::slotStateChanged( KatePartPluginListItem *item, bool b )
{
  if ( b )
    slotCurrentChanged( (TQListViewItem*)item );
}

void KatePartPluginConfigPage::slotCurrentChanged( TQListViewItem* i )
{
  KatePartPluginListItem *item = static_cast<KatePartPluginListItem *>(i);
  if ( ! item ) return;

    bool b = false;
  if ( item->isOn() )
  {

    // load this plugin, and see if it has config pages
    KTextEditor::Plugin *plugin = KTextEditor::createPlugin(TQFile::encodeName((KateFactory::self()->plugins())[item->pluginIndex()]->library()));
    if ( plugin ) {
      KTextEditor::ConfigInterfaceExtension *cie = KTextEditor::configInterfaceExtension( plugin );
      b = ( cie && cie->configPages() );
    }

  }
    btnConfigure->setEnabled( b );
}

void KatePartPluginConfigPage::slotConfigure()
{
  KatePartPluginListItem *item = static_cast<KatePartPluginListItem*>(listView->currentItem());
  KTextEditor::Plugin *plugin =
    KTextEditor::createPlugin(TQFile::encodeName((KateFactory::self()->plugins())[item->pluginIndex()]->library()));

  if ( ! plugin ) return;

  KTextEditor::ConfigInterfaceExtension *cife =
    KTextEditor::configInterfaceExtension( plugin );

  if ( ! cife )
    return;

  if ( ! cife->configPages() )
    return;

  // If we have only one page, we use a simple dialog, else an icon list type
  KDialogBase::DialogType dt =
    cife->configPages() > 1 ?
      KDialogBase::IconList :     // still untested
      KDialogBase::Plain;

  TQString name = (KateFactory::self()->plugins())[item->pluginIndex()]->name();
  KDialogBase *kd = new KDialogBase ( dt,
              i18n("Configure %1").arg( name ),
              KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
              KDialogBase::Ok,
              this );

  TQPtrList<KTextEditor::ConfigPage> editorPages;

  for (uint i = 0; i < cife->configPages (); i++)
  {
    TQWidget *page;
    if ( dt == KDialogBase::IconList )
    {
      TQStringList path;
      path.clear();
      path << cife->configPageName( i );
      page = kd->addVBoxPage( path, cife->configPageFullName (i),
                                cife->configPagePixmap(i, TDEIcon::SizeMedium) );
    }
    else
    {
      page = kd->plainPage();
      TQVBoxLayout *_l = new TQVBoxLayout( page );
      _l->setAutoAdd( true );
    }

    editorPages.append( cife->configPage( i, page ) );
  }

  if (kd->exec())
  {

    for( uint i=0; i<editorPages.count(); i++ )
    {
      editorPages.at( i )->apply();
    }
  }

  delete kd;
}
//END KatePartPluginConfigPage

//BEGIN KateHlConfigPage
KateHlConfigPage::KateHlConfigPage (TQWidget *parent, KateDocument *doc)
 : KateConfigPage (parent, "")
 , hlData (0)
 , m_doc (doc)
{
  TQVBoxLayout *layout = new TQVBoxLayout(this, 0, KDialog::spacingHint() );

  // hl chooser
  TQHBox *hbHl = new TQHBox( this );
  layout->add (hbHl);

  hbHl->setSpacing( KDialog::spacingHint() );
  TQLabel *lHl = new TQLabel( i18n("H&ighlight:"), hbHl );
  hlCombo = new TQComboBox( false, hbHl );
  lHl->setBuddy( hlCombo );
  connect( hlCombo, TQ_SIGNAL(activated(int)),
           this, TQ_SLOT(hlChanged(int)) );

  for( int i = 0; i < KateHlManager::self()->highlights(); i++) {
    if (KateHlManager::self()->hlSection(i).length() > 0)
      hlCombo->insertItem(KateHlManager::self()->hlSection(i) + TQString ("/") + KateHlManager::self()->hlNameTranslated(i));
    else
      hlCombo->insertItem(KateHlManager::self()->hlNameTranslated(i));
  }

  TQGroupBox *gbInfo = new TQGroupBox( 1, TQt::Horizontal, i18n("Information"), this );
  layout->add (gbInfo);

  // author
  TQHBox *hb1 = new TQHBox( gbInfo);
  new TQLabel( i18n("Author:"), hb1 );
  author  = new TQLabel (hb1);
  author->setTextFormat (TQt::RichText);

  // license
  TQHBox *hb2 = new TQHBox( gbInfo);
  new TQLabel( i18n("License:"), hb2 );
  license  = new TQLabel (hb2);

  TQGroupBox *gbProps = new TQGroupBox( 1, TQt::Horizontal, i18n("Properties"), this );
  layout->add (gbProps);

  // file & mime types
  TQHBox *hbFE = new TQHBox( gbProps);
  TQLabel *lFileExts = new TQLabel( i18n("File e&xtensions:"), hbFE );
  wildcards  = new TQLineEdit( hbFE );
  lFileExts->setBuddy( wildcards );

  TQHBox *hbMT = new TQHBox( gbProps );
  TQLabel *lMimeTypes = new TQLabel( i18n("MIME &types:"), hbMT);
  mimetypes = new TQLineEdit( hbMT );
  lMimeTypes->setBuddy( mimetypes );

  TQHBox *hbMT2 = new TQHBox( gbProps );
  TQLabel *lprio = new TQLabel( i18n("Prio&rity:"), hbMT2);
  priority = new KIntNumInput( hbMT2 );

  lprio->setBuddy( priority );

  TQToolButton *btnMTW = new TQToolButton(hbMT);
  btnMTW->setIconSet(TQIconSet(SmallIcon("wizard")));
  connect(btnMTW, TQ_SIGNAL(clicked()), this, TQ_SLOT(showMTDlg()));

  // download/new buttons
  TQHBox *hbBtns = new TQHBox( this );
  layout->add (hbBtns);

  ((TQBoxLayout*)hbBtns->layout())->addStretch(1); // hmm.
  hbBtns->setSpacing( KDialog::spacingHint() );
  TQPushButton *btnDl = new TQPushButton(i18n("Do&wnload..."), hbBtns);
  connect( btnDl, TQ_SIGNAL(clicked()), this, TQ_SLOT(hlDownload()) );

  int currentHl = m_doc ? m_doc->hlMode() : 0;
  hlCombo->setCurrentItem( currentHl );
  hlChanged( currentHl );

  TQWhatsThis::add( hlCombo, i18n(
        "Choose a <em>Syntax Highlight mode</em> from this list to view its "
        "properties below.") );
  TQWhatsThis::add( wildcards, i18n(
        "The list of file extensions used to determine which files to highlight "
        "using the current syntax highlight mode.") );
  TQWhatsThis::add( mimetypes, i18n(
        "The list of Mime Types used to determine which files to highlight "
        "using the current highlight mode.<p>Click the wizard button on the "
        "left of the entry field to display the MimeType selection dialog.") );
  TQWhatsThis::add( btnMTW, i18n(
        "Display a dialog with a list of all available mime types to choose from."
        "<p>The <strong>File Extensions</strong> entry will automatically be "
        "edited as well.") );
  TQWhatsThis::add( btnDl, i18n(
        "Click this button to download new or updated syntax highlight "
        "descriptions from the Kate website.") );

  layout->addStretch ();

  connect( wildcards, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( mimetypes, TQ_SIGNAL( textChanged ( const TQString & ) ), this, TQ_SLOT( slotChanged() ) );
  connect( priority, TQ_SIGNAL( valueChanged ( int ) ), this, TQ_SLOT( slotChanged() ) );
}

KateHlConfigPage::~KateHlConfigPage ()
{
}

void KateHlConfigPage::apply ()
{
  // nothing changed, no need to apply stuff
  if (!changed())
    return;
  m_changed = false;

  writeback();

  for ( TQIntDictIterator<KateHlData> it( hlDataDict ); it.current(); ++it )
    KateHlManager::self()->getHl( it.currentKey() )->setData( it.current() );

  KateHlManager::self()->getTDEConfig()->sync ();
}

void KateHlConfigPage::reload ()
{
}

void KateHlConfigPage::hlChanged(int z)
{
  writeback();

  KateHighlighting *hl = KateHlManager::self()->getHl( z );

  if (!hl)
  {
    hlData = 0;
    return;
  }

  if ( !hlDataDict.find( z ) )
    hlDataDict.insert( z, hl->getData() );

  hlData = hlDataDict.find( z );
  wildcards->setText(hlData->wildcards);
  mimetypes->setText(hlData->mimetypes);
  priority->setValue(hlData->priority);

  // split author string if needed into multiple lines !
  TQStringList l= TQStringList::split (TQRegExp("[,;]"), hl->author());
  author->setText (l.join ("<br>"));

  license->setText (hl->license());
}

void KateHlConfigPage::writeback()
{
  if (hlData)
  {
    hlData->wildcards = wildcards->text();
    hlData->mimetypes = mimetypes->text();
    hlData->priority = priority->value();
  }
}

void KateHlConfigPage::hlDownload()
{
  KateHlDownloadDialog diag(this,"hlDownload",true);
  diag.exec();
}

void KateHlConfigPage::showMTDlg()
{
  TQString text = i18n("Select the MimeTypes you want highlighted using the '%1' syntax highlight rules.\nPlease note that this will automatically edit the associated file extensions as well.").arg( hlCombo->currentText() );
  TQStringList list = TQStringList::split( TQRegExp("\\s*;\\s*"), mimetypes->text() );
  KMimeTypeChooserDialog d( i18n("Select Mime Types"), text, list, "text", this );

  if ( d.exec() == KDialogBase::Accepted ) {
    // do some checking, warn user if mime types or patterns are removed.
    // if the lists are empty, and the fields not, warn.
    wildcards->setText(d.chooser()->patterns().join(";"));
    mimetypes->setText(d.chooser()->mimeTypes().join(";"));
  }
}
//END KateHlConfigPage

//BEGIN KateHlDownloadDialog
KateHlDownloadDialog::KateHlDownloadDialog(TQWidget *parent, const char *name, bool modal)
  :KDialogBase(KDialogBase::Swallow, i18n("Highlight Download"), User1|Close, User1, parent, name, modal, true, i18n("&Install"))
{
  TQVBox* vbox = new TQVBox(this);
  setMainWidget(vbox);
  vbox->setSpacing(spacingHint());
  new TQLabel(i18n("Select the syntax highlighting files you want to update:"), vbox);
  list = new TQListView(vbox);
  list->addColumn("");
  list->addColumn(i18n("Name"));
  list->addColumn(i18n("Installed"));
  list->addColumn(i18n("Latest"));
  list->setSelectionMode(TQListView::Multi);
  list->setAllColumnsShowFocus(true);

  new TQLabel(i18n("<b>Note:</b> New versions are selected automatically."), vbox);
  actionButton (User1)->setIconSet(SmallIconSet("ok"));

  transferJob = TDEIO::get(KURL(TQString(HLDOWNLOADPATH)), true, true);
  connect(transferJob, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
    this, TQ_SLOT(listDataReceived(TDEIO::Job *, const TQByteArray &)));
//        void data( TDEIO::Job *, const TQByteArray &data);
  resize(450, 400);
}

KateHlDownloadDialog::~KateHlDownloadDialog(){}

void KateHlDownloadDialog::listDataReceived(TDEIO::Job *, const TQByteArray &data)
{
  if (!transferJob || transferJob->isErrorPage())
  {
    actionButton(User1)->setEnabled(false);
    return;
  }

  listData+=TQString(data);
  kdDebug(13000)<<TQString("CurrentListData: ")<<listData<<endl<<endl;
  kdDebug(13000)<<TQString(TQString("Data length: %1").arg(data.size()))<<endl;
  kdDebug(13000)<<TQString(TQString("listData length: %1").arg(listData.length()))<<endl;
  if (data.size()==0)
  {
    if (listData.length()>0)
    {
      TQString installedVersion;
      KateHlManager *hlm=KateHlManager::self();
      TQDomDocument doc;
      doc.setContent(listData);
      TQDomElement DocElem=doc.documentElement();
      TQDomNode n=DocElem.firstChild();
      KateHighlighting *hl = 0;

      if (n.isNull()) kdDebug(13000)<<"There is no usable childnode"<<endl;
      while (!n.isNull())
      {
        installedVersion="    --";

        TQDomElement e=n.toElement();
        if (!e.isNull())
        kdDebug(13000)<<TQString("NAME: ")<<e.tagName()<<TQString(" - ")<<e.attribute("name")<<endl;
        n=n.nextSibling();

        TQString Name=e.attribute("name");

        for (int i=0;i<hlm->highlights();i++)
        {
          hl=hlm->getHl(i);
          if (hl && hl->name()==Name)
          {
            installedVersion="    "+hl->version();
            break;
          }
          else hl = 0;
        }

        // autoselect entry if new or updated.
        TQListViewItem* entry = new TQListViewItem(
          list, "", e.attribute("name"), installedVersion,
          e.attribute("version"),e.attribute("url"));
        if (!hl || hl->version() < e.attribute("version"))
        {
          entry->setSelected(true);
          entry->setPixmap(0, SmallIcon(("knewstuff")));
        }
      }
    }
  }
}

void KateHlDownloadDialog::slotUser1()
{
  TQString destdir=TDEGlobal::dirs()->saveLocation("data","katepart/syntax/");
  for (TQListViewItem *it=list->firstChild();it;it=it->nextSibling())
  {
    if (list->isSelected(it))
    {
      KURL src(it->text(4));
      TQString filename=src.fileName(false);
      TQString dest = destdir+filename;

      TDEIO::NetAccess::download(src,dest, this);
    }
  }

  // update Config !!
  KateSyntaxDocument doc (true);
}
//END KateHlDownloadDialog

//BEGIN KateGotoLineDialog
KateGotoLineDialog::KateGotoLineDialog(TQWidget *parent, int line, int max)
  : KDialogBase(parent, 0L, true, i18n("Go to Line"), Ok | Cancel, Ok) {

  TQWidget *page = new TQWidget(this);
  setMainWidget(page);

  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );
  e1 = new KIntNumInput(line, page);
  e1->setRange(1, max);
  e1->setEditFocus(true);

  TQLabel *label = new TQLabel( e1,i18n("&Go to line:"), page );
  topLayout->addWidget(label);
  topLayout->addWidget(e1);
  topLayout->addSpacing(spacingHint()); // A little bit extra space
  topLayout->addStretch(10);
  e1->setFocus();
}

int KateGotoLineDialog::getLine() {
  return e1->value();
}
//END KateGotoLineDialog

//BEGIN KateModOnHdPrompt
KateModOnHdPrompt::KateModOnHdPrompt( KateDocument *doc,
                                      int modtype,
                                      const TQString &reason,
                                      TQWidget *parent )
  : KDialogBase( parent, "", true, "", Ok|Apply|Cancel|User1 ),
    m_doc( doc ),
    m_modtype ( modtype ),
    m_tmpfile( 0 )
{
  TQString title, btnOK, whatisok;
  if ( modtype == 3 ) // deleted
  {
    title = i18n("File Was Deleted on Disk");
    btnOK = i18n("&Save File As...");
    whatisok = i18n("Lets you select a location and save the file again.");
  } else {
    title = i18n("File Changed on Disk");
    btnOK = i18n("&Reload File");
    whatisok = i18n("Reload the file from disk. If you have unsaved changes, "
        "they will be lost.");
  }

  setButtonText( Ok, btnOK);
  setButtonText( Apply, i18n("&Ignore") );

  setButtonWhatsThis( Ok, whatisok );
  setButtonWhatsThis( Apply, i18n("Ignore the changes. You will not be prompted again.") );
  setButtonWhatsThis( Cancel, i18n("Do nothing. Next time you focus the file, "
      "or try to save it or close it, you will be prompted again.") );

  enableButtonSeparator( true );
  setCaption( title );

  TQFrame *w = makeMainWidget();
  TQVBoxLayout *lo = new TQVBoxLayout( w );
  TQHBoxLayout *lo1 = new TQHBoxLayout( lo );
  TQLabel *icon = new TQLabel( w );
  icon->setPixmap( DesktopIcon("messagebox_warning" ) );
  lo1->addWidget( icon );
  lo1->addWidget( new TQLabel( reason + "\n\n" + i18n("What do you want to do?"), w ) );

  // If the file isn't deleted, present a diff button, and a overwrite action.
  if ( modtype != 3 )
  {
    TQHBoxLayout *lo2 = new TQHBoxLayout( lo );
    TQPushButton *btnDiff = new TQPushButton( i18n("&View Difference"), w );
    lo2->addStretch( 1 );
    lo2->addWidget( btnDiff );
    connect( btnDiff, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotDiff()) );
    TQWhatsThis::add( btnDiff, i18n(
        "Calculates the difference between the editor contents and the disk "
        "file using diff(1) and opens the diff file with the default application "
        "for that.") );

    setButtonText( User1, i18n("Overwrite") );
    setButtonWhatsThis( User1, i18n("Overwrite the disk file with the editor content.") );
  }
  else
    showButton( User1, false );
}

KateModOnHdPrompt::~KateModOnHdPrompt()
{
}

void KateModOnHdPrompt::slotDiff()
{
  // Start a TDEProcess that creates a diff
  KProcIO *p = new KProcIO();
  p->setComm( TDEProcess::All );
  *p << "diff" << "-u" << "-" <<  m_doc->url().path();
  connect( p, TQ_SIGNAL(processExited(TDEProcess*)), this, TQ_SLOT(slotPDone(TDEProcess*)) );
  connect( p, TQ_SIGNAL(readReady(KProcIO*)), this, TQ_SLOT(slotPRead(KProcIO*)) );

  setCursor( WaitCursor );

  p->start( TDEProcess::NotifyOnExit, true );

  uint lastln =  m_doc->numLines();
  for ( uint l = 0; l <  lastln; l++ )
    p->writeStdin( m_doc->textLine( l ) );

  p->closeWhenDone();
}

void KateModOnHdPrompt::slotPRead( KProcIO *p)
{
  // create a file for the diff if we haven't one allready
  if ( ! m_tmpfile )
    m_tmpfile = new KTempFile();
  // put all the data we have in it
  TQString stmp;
  bool dataRead = false;
  while ( p->readln( stmp, false ) > -1 )
  {
    *m_tmpfile->textStream() << stmp << endl;
    dataRead = true;
  }

  // dominik: only ackRead(), when we *really* read data, otherwise, this slot
  // is called initity times, which leads to a crash
  if( dataRead )
    p->ackRead();
}

void KateModOnHdPrompt::slotPDone( TDEProcess *p )
{
  setCursor( ArrowCursor );
  if( ! m_tmpfile )
  {
    // dominik: there were only whitespace changes, so that the diff returned by
    // diff(1) has 0 bytes. So slotPRead() is never called, as there is
    // no data, so that m_tmpfile was never created and thus is NULL.
    // NOTE: would be nice, if we could produce a fake-diff, so that kompare
    //       tells us "The files are identical". Right now, we get an ugly
    //       "Could not parse diff output".
    m_tmpfile = new KTempFile();
  }
  m_tmpfile->close();

  if ( ! p->normalExit() /*|| p->exitStatus()*/ )
  {
    KMessageBox::sorry( this,
                        i18n("The diff command failed. Please make sure that "
                             "diff(1) is installed and in your PATH."),
                        i18n("Error Creating Diff") );
    delete m_tmpfile;
    m_tmpfile = 0;
    return;
  }

  KRun::runURL( m_tmpfile->name(), "text/x-diff", true );
  delete m_tmpfile;
  m_tmpfile = 0;
}

void KateModOnHdPrompt::slotApply()
{
  if ( KMessageBox::warningContinueCancel(
       this,
       i18n("Ignoring means that you will not be warned again (unless "
            "the disk file changes once more): if you save the document, you "
            "will overwrite the file on disk; if you do not save then the disk file "
            "(if present) is what you have."),
       i18n("You Are on Your Own"),
       KStdGuiItem::cont(),
       "kate_ignore_modonhd" ) != KMessageBox::Continue )
    return;

  done(Ignore);
}

void KateModOnHdPrompt::slotOk()
{
  done( m_modtype == 3 ? Save : Reload );
}

void KateModOnHdPrompt::slotUser1()
{
  done( Overwrite );
}

//END KateModOnHdPrompt
