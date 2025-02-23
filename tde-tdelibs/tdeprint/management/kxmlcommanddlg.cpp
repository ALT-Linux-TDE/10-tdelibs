/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kxmlcommanddlg.h"
#include "driver.h"
#include "kxmlcommand.h"

#include <tqlayout.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqcombobox.h>
#include <tqgroupbox.h>
#include <tqwidgetstack.h>
#include <tqtoolbutton.h>
#include <kpushbutton.h>
#include <tqtooltip.h>
#include <tqcheckbox.h>
#include <ktextedit.h>
#include <tqregexp.h>
#include <tqwhatsthis.h>
#include <tqapplication.h>

#include <tdelistview.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kseparator.h>
#include <tdelistbox.h>
#include <kmimetype.h>
#include <tdemessagebox.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kguiitem.h>

TQString generateId(const TQMap<TQString, DrBase*>& map)
{
	int	index(-1);
	while (map.contains(TQString::fromLatin1("item%1").arg(++index))) ;
	return TQString::fromLatin1("item%1").arg(index);
}

TQListViewItem* findPrev(TQListViewItem *item)
{
	TQListViewItem	*prev = item->itemAbove();
	while (prev && prev->depth() > item->depth())
		prev = prev->itemAbove();
	if (prev && prev->depth() == item->depth())
		return prev;
	else
		return 0;
}

TQListViewItem* findNext(TQListViewItem *item)
{
	TQListViewItem	*next = item->itemBelow();
	while (next && next->depth() > item->depth())
		next = next->itemBelow();
	if (next && next->depth() == item->depth())
		return next;
	else
		return 0;
}

KXmlCommandAdvancedDlg::KXmlCommandAdvancedDlg(TQWidget *parent, const char *name)
: TQWidget(parent, name)
{
	m_xmlcmd = 0;

	m_command = new TQLineEdit(this);
	m_view = new TDEListView(this);
	m_view->addColumn("");
	m_view->header()->hide();
	m_view->setSorting(-1);
	m_apply = new TQToolButton(this);
	m_apply->setIconSet( TQApplication::reverseLayout()? SmallIconSet( "forward" ) : SmallIconSet("back"));
	m_addgrp = new TQToolButton(this);
	m_addgrp->setIconSet(SmallIconSet("folder"));
	m_addopt = new TQToolButton(this);
	m_addopt->setIconSet(SmallIconSet("text-x-generic"));
	m_delopt = new TQToolButton(this);
	m_delopt->setIconSet(SmallIconSet("edit-delete"));
	m_up = new TQToolButton(this);
	m_up->setIconSet(SmallIconSet("go-up"));
	m_down = new TQToolButton(this);
	m_down->setIconSet(SmallIconSet("go-down"));
	m_dummy = new TQWidget(this);
	m_desc = new TQLineEdit(m_dummy);
	m_name = new TQLineEdit(m_dummy);
	m_type = new TQComboBox(m_dummy);
	m_type->insertItem(i18n("String"));
	m_type->insertItem(i18n("Integer"));
	m_type->insertItem(i18n("Float"));
	m_type->insertItem(i18n("List"));
	m_type->insertItem(i18n("Boolean"));
	m_format = new TQLineEdit(m_dummy);
	m_default = new TQLineEdit(m_dummy);
	TQLabel	*m_namelab = new TQLabel(i18n("&Name:"), m_dummy);
	TQLabel	*m_desclab = new TQLabel(i18n("&Description:"), m_dummy);
	TQLabel	*m_formatlab = new TQLabel(i18n("&Format:"), m_dummy);
	TQLabel	*m_typelab = new TQLabel(i18n("&Type:"), m_dummy);
	TQLabel	*m_defaultlab = new TQLabel(i18n("Default &value:"), m_dummy);
	TQLabel	*m_commandlab = new TQLabel(i18n("Co&mmand:"), this);
	m_namelab->setBuddy(m_name);
	m_desclab->setBuddy(m_desc);
	m_formatlab->setBuddy(m_format);
	m_typelab->setBuddy(m_type);
	m_defaultlab->setBuddy(m_default);
	m_commandlab->setBuddy(m_command);
	m_persistent = new TQCheckBox( i18n( "&Persistent option" ), m_dummy );

	TQGroupBox	*gb = new TQGroupBox(0, TQt::Horizontal, i18n("Va&lues"), m_dummy);
	m_stack = new TQWidgetStack(gb);
	TQWidget	*w1 = new TQWidget(m_stack), *w2 = new TQWidget(m_stack), *w3 = new TQWidget(m_stack);
	m_stack->addWidget(w1, 1);
	m_stack->addWidget(w2, 2);
	m_stack->addWidget(w3, 3);
	m_edit1 = new TQLineEdit(w1);
	m_edit2 = new TQLineEdit(w1);
	TQLabel	*m_editlab1 = new TQLabel(i18n("Minimum v&alue:"), w1);
	TQLabel	*m_editlab2 = new TQLabel(i18n("Ma&ximum value:"), w1);
	m_editlab1->setBuddy(m_edit1);
	m_editlab2->setBuddy(m_edit2);
	m_values = new TDEListView(w2);
	m_values->addColumn(i18n("Name"));
	m_values->addColumn(i18n("Description"));
	m_values->setAllColumnsShowFocus(true);
	m_values->setSorting(-1);
	m_values->setMaximumHeight(110);
	m_addval = new TQToolButton(w2);
	m_addval->setIconSet(SmallIconSet("edit-copy"));
	m_delval = new TQToolButton(w2);
	m_delval->setIconSet(SmallIconSet("edit-delete"));
	TQToolTip::add(m_addval, i18n("Add value"));
	TQToolTip::add(m_delval, i18n("Delete value"));

	TQToolTip::add(m_apply, i18n("Apply changes"));
	TQToolTip::add(m_addgrp, i18n("Add group"));
	TQToolTip::add(m_addopt, i18n("Add option"));
	TQToolTip::add(m_delopt, i18n("Delete item"));
	TQToolTip::add(m_up, i18n("Move up"));
	TQToolTip::add(m_down, i18n("Move down"));

	KSeparator	*sep1 = new KSeparator(KSeparator::HLine, m_dummy);

	TQGroupBox	*gb_input = new TQGroupBox(0, TQt::Horizontal, i18n("&Input From"), this);
	TQGroupBox	*gb_output = new TQGroupBox(0, TQt::Horizontal, i18n("O&utput To"), this);
	TQLabel	*m_inputfilelab = new TQLabel(i18n("File:"), gb_input);
	TQLabel	*m_inputpipelab = new TQLabel(i18n("Pipe:"), gb_input);
	TQLabel	*m_outputfilelab = new TQLabel(i18n("File:"), gb_output);
	TQLabel	*m_outputpipelab = new TQLabel(i18n("Pipe:"), gb_output);
	m_inputfile = new TQLineEdit(gb_input);
	m_inputpipe = new TQLineEdit(gb_input);
	m_outputfile = new TQLineEdit(gb_output);
	m_outputpipe = new TQLineEdit(gb_output);

	m_comment = new KTextEdit( this );
	m_comment->setTextFormat(TQt::RichText );
	m_comment->setReadOnly(true);
	TQLabel *m_commentlab = new TQLabel( i18n( "Comment:" ), this );

	TQVBoxLayout	*l2 = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	TQHBoxLayout	*l3 = new TQHBoxLayout(0, 0, KDialog::spacingHint());
	TQVBoxLayout	*l7 = new TQVBoxLayout(0, 0, 0);
	l2->addLayout(l3, 0);
	l3->addWidget(m_commandlab);
	l3->addWidget(m_command);
	TQHBoxLayout	*l0 = new TQHBoxLayout(0, 0, KDialog::spacingHint());
	TQGridLayout	*l10 = new TQGridLayout(0, 2, 2, 0, KDialog::spacingHint());
	l2->addLayout(l0, 1);
	l0->addLayout(l10);
	l10->addMultiCellWidget(m_view, 0, 0, 0, 1);
	l10->addWidget(gb_input, 1, 0);
	l10->addWidget(gb_output, 1, 1);
	l10->setRowStretch(0, 1);
	l0->addLayout(l7);
	l7->addWidget(m_apply);
	l7->addSpacing(5);
	l7->addWidget(m_addgrp);
	l7->addWidget(m_addopt);
	l7->addWidget(m_delopt);
	l7->addSpacing(5);
	l7->addWidget(m_up);
	l7->addWidget(m_down);
	l7->addStretch(1);
	l0->addWidget(m_dummy, 1);
	TQGridLayout	*l1 = new TQGridLayout(m_dummy, 9, 2, 0, KDialog::spacingHint());
	l1->addWidget(m_desclab, 0, 0, TQt::AlignRight|TQt::AlignVCenter);
	l1->addWidget(m_desc, 0, 1);
	l1->addMultiCellWidget(sep1, 1, 1, 0, 1);
	l1->addWidget(m_namelab, 2, 0, TQt::AlignRight|TQt::AlignVCenter);
	l1->addWidget(m_name, 2, 1);
	l1->addWidget(m_typelab, 3, 0, TQt::AlignRight|TQt::AlignVCenter);
	l1->addWidget(m_type, 3, 1);
	l1->addWidget(m_formatlab, 4, 0, TQt::AlignRight|TQt::AlignVCenter);
	l1->addWidget(m_format, 4, 1);
	l1->addWidget(m_defaultlab, 5, 0, TQt::AlignRight|TQt::AlignVCenter);
	l1->addWidget(m_default, 5, 1);
	l1->addWidget( m_persistent, 6, 1 );
	l1->addMultiCellWidget(gb, 7, 7, 0, 1);
	l1->setRowStretch(8, 1);

	TQHBoxLayout	*l4 = new TQHBoxLayout(w2, 0, KDialog::spacingHint());
	l4->addWidget(m_values);
	TQVBoxLayout	*l6 = new TQVBoxLayout(0, 0, 0);
	l4->addLayout(l6);
	l6->addWidget(m_addval);
	l6->addWidget(m_delval);
	l6->addStretch(1);
	TQGridLayout	*l5 = new TQGridLayout(w1, 3, 2, 0, KDialog::spacingHint());
	l5->setRowStretch(2, 1);
	l5->addWidget(m_editlab1, 0, 0, TQt::AlignRight|TQt::AlignVCenter);
	l5->addWidget(m_editlab2, 1, 0, TQt::AlignRight|TQt::AlignVCenter);
	l5->addWidget(m_edit1, 0, 1);
	l5->addWidget(m_edit2, 1, 1);

	TQGridLayout	*l8 = new TQGridLayout(gb_input->layout(), 2, 2,
		KDialog::spacingHint());
	TQGridLayout	*l9 = new TQGridLayout(gb_output->layout(), 2, 2,
		KDialog::spacingHint());
	l8->addWidget(m_inputfilelab, 0, 0);
	l8->addWidget(m_inputpipelab, 1, 0);
	l8->addWidget(m_inputfile, 0, 1);
	l8->addWidget(m_inputpipe, 1, 1);
	l9->addWidget(m_outputfilelab, 0, 0);
	l9->addWidget(m_outputpipelab, 1, 0);
	l9->addWidget(m_outputfile, 0, 1);
	l9->addWidget(m_outputpipe, 1, 1);

	TQVBoxLayout	*l11 = new TQVBoxLayout(gb->layout());
	l11->addWidget(m_stack);

	TQVBoxLayout *l12 = new TQVBoxLayout( 0, 0, 0 );
	l2->addSpacing( 10 );
	l2->addLayout( l12 );
	l12->addWidget( m_commentlab );
	l12->addWidget( m_comment );

	connect(m_view, TQ_SIGNAL(selectionChanged(TQListViewItem*)), TQ_SLOT(slotSelectionChanged(TQListViewItem*)));
	connect(m_values, TQ_SIGNAL(selectionChanged(TQListViewItem*)), TQ_SLOT(slotValueSelected(TQListViewItem*)));
	connect(m_type, TQ_SIGNAL(activated(int)), TQ_SLOT(slotTypeChanged(int)));
	connect(m_addval, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddValue()));
	connect(m_delval, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveValue()));
	connect(m_apply, TQ_SIGNAL(clicked()), TQ_SLOT(slotApplyChanges()));
	connect(m_addgrp, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddGroup()));
	connect(m_addopt, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddOption()));
	connect(m_delopt, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveItem()));
	connect(m_up, TQ_SIGNAL(clicked()), TQ_SLOT(slotMoveUp()));
	connect(m_down, TQ_SIGNAL(clicked()), TQ_SLOT(slotMoveDown()));
	connect(m_command, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotCommandChanged(const TQString&)));
	connect(m_view, TQ_SIGNAL(itemRenamed(TQListViewItem*,int)), TQ_SLOT(slotOptionRenamed(TQListViewItem*,int)));
	connect(m_desc, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_name, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_format, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_default, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_edit1, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_edit2, TQ_SIGNAL(textChanged(const TQString&)), TQ_SLOT(slotChanged()));
	connect(m_type, TQ_SIGNAL(activated(int)), TQ_SLOT(slotChanged()));
	connect(m_addval, TQ_SIGNAL(clicked()), TQ_SLOT(slotChanged()));
	connect(m_delval, TQ_SIGNAL(clicked()), TQ_SLOT(slotChanged()));
	connect( m_persistent, TQ_SIGNAL( toggled(bool) ), TQ_SLOT( slotChanged() ) );
	m_dummy->setEnabled(false);
	viewItem(0);

	/**
	 * Add some short help for the normal (non expert) user
	 */
	TQWhatsThis::add( m_name, i18n(
				"An identification string. Use only alphanumeric characters except spaces. "
				"The string <b>__root__</b> is reserved for internal use." ) );
	TQWhatsThis::add( m_namelab, TQWhatsThis::textFor( m_name ) );
	TQWhatsThis::add( m_desc, i18n(
				"A description string. This string is shown in the interface, and should "
				"be explicit enough about the role of the corresponding option." ) );
	TQWhatsThis::add( m_desclab, TQWhatsThis::textFor( m_desc ) );
	TQWhatsThis::add( m_type, i18n(
				"The type of the option. This determines how the option is presented "
				"graphically to the user." ) );
	TQWhatsThis::add( m_typelab, TQWhatsThis::textFor( m_type ) );
	TQWhatsThis::add( m_format, i18n(
				"The format of the option. This determines how the option is formatted "
				"for inclusion in the global command line. The tag <b>%value</b> can be used "
				"to represent the user selection. This tag will be replaced at run-time by a "
				"string representation of the option value." ) );
	TQWhatsThis::add( m_formatlab, TQWhatsThis::textFor( m_format ) );
	TQWhatsThis::add( m_default, i18n(
				"The default value of the option. For non persistent options, nothing is "
				"added to the command line if the option has that default value. If this "
				"value does not correspond to the actual default value of the underlying "
				"utility, make the option persistent to avoid unwanted effects." ) );
	TQWhatsThis::add( m_defaultlab, TQWhatsThis::textFor( m_default ) );
	TQWhatsThis::add( m_persistent, i18n(
				"Make the option persistent. A persistent option is always written to the "
				"command line, whatever its value. This is useful when the chosen default "
				"value does not match with the actual default value of the underlying utility." ) );
	TQWhatsThis::add( m_command, i18n(
				"The full command line to execute the associated underlying utility. This "
				"command line is based on a mechanism of tags that are replaced at run-time. "
				"The supported tags are:<ul>"
				"<li><b>%filterargs</b>: command options</li>"
				"<li><b>%filterinput</b>: input specification</li>"
				"<li><b>%filteroutput</b>: output specification</li>"
				"<li><b>%psu</b>: the page size in upper case</li>"
				"<li><b>%psl</b>: the page size in lower case</li></ul>" ) );
	TQWhatsThis::add( m_commandlab, TQWhatsThis::textFor( m_command ) );
	TQWhatsThis::add( m_inputfile, i18n(
				"Input specification when the underlying utility reads input data from a file. Use "
				"the tag <b>%in</b> to represent the input filename." ) );
	TQWhatsThis::add( m_inputfilelab, TQWhatsThis::textFor( m_inputfile ) );
	TQWhatsThis::add( m_outputfile, i18n(
				"Output specification when the underlying utility writes output data to a file. Use "
				"the tag <b>%out</b> to represent the output filename." ) );
	TQWhatsThis::add( m_outputfilelab, TQWhatsThis::textFor( m_outputfile ) );
	TQWhatsThis::add( m_inputpipe, i18n(
				"Input specification when the underlying utility reads input data from its "
				"standard input." ) );
	TQWhatsThis::add( m_inputpipelab, TQWhatsThis::textFor( m_inputpipe ) );
	TQWhatsThis::add( m_outputpipe, i18n(
				"Output specification when the underlying utility writes output data to its "
				"standard output." ) );
	TQWhatsThis::add( m_outputpipelab, TQWhatsThis::textFor( m_outputpipe ) );
	TQWhatsThis::add( m_comment, i18n(
				"A comment about the underlying utility, which can be viewed by the user "
				"from the interface. This comment string supports basic HTML tags like "
				"&lt;a&gt;, &lt;b&gt; or &lt;i&gt;." ) );
	TQWhatsThis::add( m_commentlab, TQWhatsThis::textFor( m_comment ) );

	resize(660, 200);
}

KXmlCommandAdvancedDlg::~KXmlCommandAdvancedDlg()
{
	if (m_opts.count() > 0)
	{
		kdDebug() << "KXmlCommandAdvancedDlg: " << m_opts.count() << " items remaining" << endl;
		for (TQMap<TQString,DrBase*>::ConstIterator it=m_opts.begin(); it!=m_opts.end(); ++it)
		{
			//kdDebug() << "Item: name=" << (*it)->name() << endl;
			delete (*it);
		}
	}
}

void KXmlCommandAdvancedDlg::setCommand(KXmlCommand *xmlcmd)
{
	m_xmlcmd = xmlcmd;
	if (m_xmlcmd)
		parseXmlCommand(m_xmlcmd);
}

void KXmlCommandAdvancedDlg::parseXmlCommand(KXmlCommand *xmlcmd)
{
	m_view->clear();
	TQListViewItem	*root = new TQListViewItem(m_view, xmlcmd->name(), xmlcmd->name());
	DrMain	*driver = xmlcmd->driver();

	root->setPixmap(0, SmallIcon("document-print"));
	root->setOpen(true);
	if (driver)
	{
		DrMain	*clone = driver->cloneDriver();
		if (!clone->get("text").isEmpty())
			root->setText(0, clone->get("text"));
		root->setText(1, "__root__");
		clone->setName("__root__");
		m_opts["__root__"] = clone;
		parseGroupItem(clone, root);
		clone->flatten();
	}
	m_command->setText(xmlcmd->command());
	m_inputfile->setText(xmlcmd->io(true, false));
	m_inputpipe->setText(xmlcmd->io(true, true));
	m_outputfile->setText(xmlcmd->io(false, false));
	m_outputpipe->setText(xmlcmd->io(false, true));
	m_comment->setText( xmlcmd->comment() );

	viewItem(0);
}

void KXmlCommandAdvancedDlg::parseGroupItem(DrGroup *grp, TQListViewItem *parent)
{
	TQListViewItem	*item(0);

	TQPtrListIterator<DrGroup>	git(grp->groups());
	for (; git.current(); ++git)
	{
		TQString	namestr = git.current()->name();
		if (namestr.isEmpty())
		{
			namestr = "group_"+kapp->randomString(4);
		}
		git.current()->setName(namestr);
		item = new TQListViewItem(parent, item, git.current()->get("text"), git.current()->name());
		item->setPixmap(0, SmallIcon("folder"));
		item->setOpen(true);
		item->setRenameEnabled(0, true);
		parseGroupItem(git.current(), item);
		m_opts[namestr] = git.current();
	}

	TQPtrListIterator<DrBase>	oit(grp->options());
	for (; oit.current(); ++oit)
	{
		TQString	namestr = oit.current()->name().mid(m_xmlcmd->name().length()+6);
		if (namestr.isEmpty())
		{
			namestr = "option_"+kapp->randomString(4);
		}
		oit.current()->setName(namestr);
		item = new TQListViewItem(parent, item, oit.current()->get("text"), namestr);
		item->setPixmap(0, SmallIcon("text-x-generic"));
		item->setRenameEnabled(0, true);
		m_opts[namestr] = oit.current();
	}
}

void KXmlCommandAdvancedDlg::slotSelectionChanged(TQListViewItem *item)
{
	if (item && item->depth() == 0)
		item = 0;
	viewItem(item);
}

void KXmlCommandAdvancedDlg::viewItem(TQListViewItem *item)
{
	m_dummy->setEnabled((item != 0));
	m_name->setText("");
	m_desc->setText("");
	m_format->setText("");
	m_default->setText("");
	m_values->clear();
	m_edit1->setText("");
	m_edit2->setText("");
	m_persistent->setChecked( false );
	int	typeId(-1);
	if (item)
	{
		m_name->setText(item->text(1));
		m_desc->setText(item->text(0));

		DrBase	*opt = (m_opts.contains(item->text(1)) ? m_opts[item->text(1)] : 0);
		if (opt)
		{
			bool	isgroup = (opt->type() < DrBase::String);
			if (!isgroup)
			{
				m_type->setCurrentItem(opt->type() - DrBase::String);
				typeId = m_type->currentItem();
				m_format->setText(opt->get("format"));
				m_default->setText(opt->get("default"));
			}
			m_type->setEnabled(!isgroup);
			m_default->setEnabled(!isgroup);
			m_format->setEnabled(!isgroup);
			m_stack->setEnabled(!isgroup);

			switch (opt->type())
			{
				case DrBase::Float:
				case DrBase::Integer:
					m_edit1->setText(opt->get("minval"));
					m_edit2->setText(opt->get("maxval"));
					break;
				case DrBase::Boolean:
				case DrBase::List:
					{
						TQPtrListIterator<DrBase>	it(*(static_cast<DrListOption*>(opt)->choices()));
						TQListViewItem	*item(0);
						for (; it.current(); ++it)
						{
							item = new TQListViewItem(m_values, item, it.current()->name(), it.current()->get("text"));
							item->setRenameEnabled(0, true);
							item->setRenameEnabled(1, true);
						}
						break;
					}
				default:
					break;
			}

			m_addgrp->setEnabled(isgroup);
			m_addopt->setEnabled(isgroup);

			TQListViewItem	*prevItem = findPrev(item), *nextItem = findNext(item);
			DrBase	*prevOpt = (prevItem && m_opts.contains(prevItem->text(1)) ? m_opts[prevItem->text(1)] : 0);
			DrBase	*nextOpt = (nextItem && m_opts.contains(nextItem->text(1)) ? m_opts[nextItem->text(1)] : 0);
			m_up->setEnabled(prevOpt && !(prevOpt->type() < DrBase::String && opt->type() >= DrBase::String));
			m_down->setEnabled(nextOpt && !(isgroup && nextOpt->type() >= DrBase::String));

			m_persistent->setChecked( opt->get( "persistent" ) == "1" );
		}

		m_delopt->setEnabled(true);
		m_dummy->setEnabled(opt);
	}
	else
	{
		m_delopt->setEnabled(false);
		m_addopt->setEnabled(m_view->currentItem() && m_view->isEnabled());
		m_addgrp->setEnabled(m_view->currentItem() && m_view->isEnabled());
		m_up->setEnabled(false);
		m_down->setEnabled(false);
	}
	slotTypeChanged(typeId);
	m_apply->setEnabled(false);
}

void KXmlCommandAdvancedDlg::slotTypeChanged(int ID)
{
	int	wId(3);
	ID += DrBase::String;
	switch (ID)
	{
		case DrBase::Float:
		case DrBase::Integer:
			wId = 1;
			break;
		case DrBase::Boolean:
		case DrBase::List:
			wId = 2;
			slotValueSelected(m_values->currentItem());
			break;
	}
	m_stack->raiseWidget(wId);
}

void KXmlCommandAdvancedDlg::slotAddValue()
{
	TQListViewItem	*item = new TQListViewItem(m_values, m_values->lastItem(), i18n("Name"), i18n("Description"));
	item->setRenameEnabled(0, true);
	item->setRenameEnabled(1, true);
	m_values->ensureItemVisible(item);
	slotValueSelected(item);
	item->startRename(0);
}

void KXmlCommandAdvancedDlg::slotRemoveValue()
{
	TQListViewItem	*item = m_values->currentItem();
	if (item)
		delete item;
	slotValueSelected(m_values->currentItem());
}

void KXmlCommandAdvancedDlg::slotApplyChanges()
{
	TQListViewItem	*item = m_view->currentItem();
	if (item)
	{
		if (m_name->text().isEmpty() || m_name->text() == "__root__")
		{
			KMessageBox::error(this, i18n("Invalid identification name. Empty strings and \"__root__\" are not allowed."));
			return;
		}

		m_apply->setEnabled(false);

		DrBase	*opt = (m_opts.contains(item->text(1)) ? m_opts[item->text(1)] : 0);
		m_opts.remove(item->text(1));
		delete opt;

		// update tree item
		item->setText(0, m_desc->text());
		item->setText(1, m_name->text());

		// recreate option
		if (m_type->isEnabled())
		{
			int	type = m_type->currentItem() + DrBase::String;
			switch (type)
			{
				case DrBase::Integer:
				case DrBase::Float:
					if (type == DrBase::Integer)
						opt = new DrIntegerOption;
					else
						opt = new DrFloatOption;
					opt->set("minval", m_edit1->text());
					opt->set("maxval", m_edit2->text());
					break;
				case DrBase::List:
				case DrBase::Boolean:
					{
						if (type == DrBase::List)
							opt = new DrListOption;
						else
							opt = new DrBooleanOption;
						DrListOption	*lopt = static_cast<DrListOption*>(opt);
						TQListViewItem	*item = m_values->firstChild();
						while (item)
						{
							DrBase	*choice = new DrBase;
							choice->setName(item->text(0));
							choice->set("text", item->text(1));
							lopt->addChoice(choice);
							item = item->nextSibling();
						}
						break;
					}
				case DrBase::String:
					opt = new DrStringOption;
					break;

			}
			opt->set("format", m_format->text());
			opt->set("default", m_default->text());
			opt->setValueText(opt->get("default"));
		}
		else
			opt = new DrGroup;

		opt->setName((m_name->text().isEmpty() ? generateId(m_opts) : m_name->text()));
		opt->set("text", m_desc->text());
		opt->set( "persistent", m_persistent->isChecked() ? "1" : "0" );

		m_opts[opt->name()] = opt;
	}
}

void KXmlCommandAdvancedDlg::slotChanged()
{
	m_apply->setEnabled(true);
}

void KXmlCommandAdvancedDlg::slotAddGroup()
{
	if (m_view->currentItem())
	{
		TQString	ID = generateId(m_opts);

		DrGroup	*grp = new DrGroup;
		grp->setName(ID);
		grp->set("text", i18n("New Group"));
		m_opts[ID] = grp;

		TQListViewItem	*item = new TQListViewItem(m_view->currentItem(), i18n("New Group"), ID);
		item->setRenameEnabled(0, true);
		item->setPixmap(0, SmallIcon("folder"));
		m_view->ensureItemVisible(item);
		item->startRename(0);
	}
}

void KXmlCommandAdvancedDlg::slotAddOption()
{
	if (m_view->currentItem())
	{
		TQString	ID = generateId(m_opts);

		DrBase	*opt = new DrStringOption;
		opt->setName(ID);
		opt->set("text", i18n("New Option"));
		m_opts[ID] = opt;

		TQListViewItem	*item = new TQListViewItem(m_view->currentItem(), i18n("New Option"), ID);
		item->setRenameEnabled(0, true);
		item->setPixmap(0, SmallIcon("text-x-generic"));
		m_view->ensureItemVisible(item);
		item->startRename(0);
	}
}

void KXmlCommandAdvancedDlg::slotRemoveItem()
{
	TQListViewItem	*item = m_view->currentItem();
	if (item)
	{
		TQListViewItem	*newCurrent(item->nextSibling());
		if (!newCurrent)
			newCurrent = item->parent();
		removeItem(item);
		delete item;
		m_view->setSelected(newCurrent, true);
	}
}

void KXmlCommandAdvancedDlg::removeItem(TQListViewItem *item)
{
	delete m_opts[item->text(1)];
	m_opts.remove(item->text(1));
	TQListViewItem	*child = item->firstChild();
	while (child && item)
	{
		removeItem(child);
                if ( item )
                    item = item->nextSibling();
	}
}

void KXmlCommandAdvancedDlg::slotMoveUp()
{
	TQListViewItem	*item = m_view->currentItem(), *prev = 0;
	if (item && (prev=findPrev(item)))
	{
		TQListViewItem	*after(0);
		if ((after=findPrev(prev)) != 0)
			item->moveItem(after);
		else
		{
			TQListViewItem	*parent = item->parent();
			parent->takeItem(item);
			parent->insertItem(item);
		}
		m_view->setSelected(item, true);
		slotSelectionChanged(item);
	}
}

void KXmlCommandAdvancedDlg::slotMoveDown()
{
	TQListViewItem	*item = m_view->currentItem(), *next = 0;
	if (item && (next=findNext(item)))
	{
		item->moveItem(next);
		m_view->setSelected(item, true);
		slotSelectionChanged(item);
	}
}

void KXmlCommandAdvancedDlg::slotCommandChanged(const TQString& cmd)
{
	m_inputfile->parentWidget()->setEnabled(cmd.find("%filterinput") != -1);
	m_outputfile->parentWidget()->setEnabled(cmd.find("%filteroutput") != -1);
	m_view->setEnabled(cmd.find("%filterargs") != -1);
	m_name->parentWidget()->setEnabled(m_view->isEnabled());
	slotSelectionChanged((m_view->isEnabled() ? m_view->currentItem() : 0));
	m_view->setOpen(m_view->firstChild(), m_view->isEnabled());
}

void KXmlCommandAdvancedDlg::slotValueSelected(TQListViewItem *item)
{
	m_addval->setEnabled(m_type->currentItem() != 4 || m_values->childCount() < 2);
	m_delval->setEnabled(item != 0);
}

void KXmlCommandAdvancedDlg::slotOptionRenamed(TQListViewItem *item, int)
{
	if (item && m_opts.contains(item->text(1)))
	{
		DrBase	*opt = m_opts[item->text(1)];
		opt->set("text", item->text(0));
		slotSelectionChanged(item);
	}
}

void KXmlCommandAdvancedDlg::recreateGroup(TQListViewItem *item, DrGroup *grp)
{
	if (!item)
		return;

	TQListViewItem	*child = item->firstChild();
	while (child)
	{
		DrBase	*opt = (m_opts.contains(child->text(1)) ? m_opts[child->text(1)] : 0);
		if (opt)
		{
			if (opt->type() == DrBase::Group)
			{
				DrGroup	*childGroup = static_cast<DrGroup*>(opt);
				recreateGroup(child, childGroup);
				grp->addGroup(childGroup);
			}
			else
			{
				opt->setName("_kde-"+m_xmlcmd->name()+"-"+opt->name());
				grp->addOption(opt);
			}
			m_opts.remove(child->text(1));
		}
		child = child->nextSibling();
	}
}

bool KXmlCommandAdvancedDlg::editCommand(KXmlCommand *xmlcmd, TQWidget *parent)
{
	if (!xmlcmd)
		return false;

	KDialogBase	dlg(parent, 0, true, i18n("Command Edit for %1").arg(xmlcmd->name()), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, false);
	KXmlCommandAdvancedDlg	*xmldlg = new KXmlCommandAdvancedDlg(&dlg);
	dlg.setMainWidget(xmldlg);
	//dlg.enableButton(KDialogBase::Ok, false);
	xmldlg->setCommand(xmlcmd);
	if (dlg.exec())
	{
		xmlcmd->setCommand(xmldlg->m_command->text());
		xmlcmd->setIo(xmldlg->m_inputfile->text(), true, false);
		xmlcmd->setIo(xmldlg->m_inputpipe->text(), true, true);
		xmlcmd->setIo(xmldlg->m_outputfile->text(), false, false);
		xmlcmd->setIo(xmldlg->m_outputpipe->text(), false, true);
		xmlcmd->setComment( xmldlg->m_comment->text().replace( TQRegExp( "\n" ), " " ) );

		// need to recreate the driver tree structure
		DrMain	*driver = (xmldlg->m_opts.contains("__root__") ? static_cast<DrMain*>(xmldlg->m_opts["__root__"]) : 0);
		if (!driver && xmldlg->m_opts.count() > 0)
		{
			kdDebug() << "KXmlCommandAdvancedDlg: driver structure not found, creating one" << endl;
			driver = new DrMain;
			driver->setName(xmlcmd->name());
		}
		xmldlg->recreateGroup(xmldlg->m_view->firstChild(), driver);
		xmldlg->m_opts.remove("__root__");
		xmlcmd->setDriver(driver);

		// remaining options will be removed in destructor

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------

KXmlCommandDlg::KXmlCommandDlg(TQWidget *parent, const char *name)
: KDialogBase(parent, name, true, TQString::null, Ok|Cancel|Details, Ok, true)
{
	setButtonText(Details, i18n("&Mime Type Settings"));
	m_cmd = 0;

	TQWidget	*dummy = new TQWidget(this, "TopDetail");
	TQWidget	*topmain = new TQWidget(this, "TopMain");

	TQGroupBox	*m_gb1 = new TQGroupBox(0, TQt::Horizontal, i18n("Supported &Input Formats"), dummy);
	TQGroupBox	*m_gb2 = new TQGroupBox(0, TQt::Horizontal, i18n("Requirements"), topmain);

	m_description = new TQLineEdit(topmain);
	m_idname = new TQLabel(topmain);
	m_requirements = new TDEListView(m_gb2);
	m_requirements->addColumn("");
	m_requirements->header()->hide();
	m_addreq = new TQToolButton(m_gb2);
	m_addreq->setIconSet(SmallIconSet("document-new"));
	m_removereq = new TQToolButton(m_gb2);
	m_removereq->setIconSet(SmallIconSet("edit-delete"));
	TQPushButton	*m_edit = new KPushButton(KGuiItem(i18n("&Edit Command..."), "edit"), topmain);
	m_mimetype = new TQComboBox(dummy);
	m_availablemime = new TDEListBox(m_gb1);
	m_selectedmime = new TDEListBox(m_gb1);
	m_addmime = new TQToolButton(m_gb1);
	m_addmime->setIconSet(TQApplication::reverseLayout()? SmallIconSet("forward") : SmallIconSet("back"));
	m_removemime = new TQToolButton(m_gb1);
	m_removemime->setIconSet(TQApplication::reverseLayout()? SmallIconSet("back" ) : SmallIconSet("forward"));
	m_gb2->setMinimumWidth(380);
	m_gb1->setMinimumHeight(180);
	m_requirements->setMaximumHeight(80);
	m_removereq->setEnabled(false);
	m_addmime->setEnabled(false);
	m_removemime->setEnabled(false);

	TQLabel	*m_desclab = new TQLabel(i18n("&Description:"), topmain);
	m_desclab->setBuddy(m_description);
	TQLabel	*m_mimetypelab = new TQLabel(i18n("Output &format:"), dummy);
	m_mimetypelab->setBuddy(m_mimetype);
	TQLabel	*m_idnamelab = new TQLabel(i18n("ID name:"), topmain);

	TQFont	f(m_idname->font());
	f.setBold(true);
	m_idname->setFont(f);

	KSeparator	*sep1 = new KSeparator(TQFrame::HLine, dummy);

	TQVBoxLayout	*l0 = new TQVBoxLayout(topmain, 0, 10);
	TQGridLayout	*l5 = new TQGridLayout(0, 2, 2, 0, 5);
	l0->addLayout(l5);
	l5->addWidget(m_idnamelab, 0, 0);
	l5->addWidget(m_idname, 0, 1);
	l5->addWidget(m_desclab, 1, 0);
	l5->addWidget(m_description, 1, 1);
	l0->addWidget(m_gb2);
	TQHBoxLayout	*l3 = new TQHBoxLayout(0, 0, 0);
	l0->addLayout(l3);
	l3->addWidget(m_edit);
	l3->addStretch(1);

	TQVBoxLayout	*l7 = new TQVBoxLayout(dummy, 0, 10);
	TQHBoxLayout	*l6 = new TQHBoxLayout(0, 0, 5);
	l7->addWidget(sep1);
	l7->addLayout(l6);
	l6->addWidget(m_mimetypelab, 0);
	l6->addWidget(m_mimetype, 1);
	l7->addWidget(m_gb1);
	TQGridLayout	*l2 = new TQGridLayout(m_gb1->layout(), 4, 3, 10);
	l2->addMultiCellWidget(m_availablemime, 0, 3, 2, 2);
	l2->addMultiCellWidget(m_selectedmime, 0, 3, 0, 0);
	l2->addWidget(m_addmime, 1, 1);
	l2->addWidget(m_removemime, 2, 1);
	l2->setRowStretch(0, 1);
	l2->setRowStretch(3, 1);
	TQHBoxLayout	*l4 = new TQHBoxLayout(m_gb2->layout(), 10);
	l4->addWidget(m_requirements);
	TQVBoxLayout	*l8 = new TQVBoxLayout(0, 0, 0);
	l4->addLayout(l8);
	l8->addWidget(m_addreq);
	l8->addWidget(m_removereq);
	l8->addStretch(1);

	connect(m_addmime, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddMime()));
	connect(m_removemime, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveMime()));
	connect(m_edit, TQ_SIGNAL(clicked()), TQ_SLOT(slotEditCommand()));
	connect(m_requirements, TQ_SIGNAL(selectionChanged(TQListViewItem*)), TQ_SLOT(slotReqSelected(TQListViewItem*)));
	connect(m_availablemime, TQ_SIGNAL(selectionChanged(TQListBoxItem*)), TQ_SLOT(slotAvailableSelected(TQListBoxItem*)));
	connect(m_selectedmime, TQ_SIGNAL(selectionChanged(TQListBoxItem*)), TQ_SLOT(slotSelectedSelected(TQListBoxItem*)));
	connect(m_addreq, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddReq()));
	connect(m_removereq, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveReq()));

	KMimeType::List	list = KMimeType::allMimeTypes();
	for (TQValueList<KMimeType::Ptr>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		TQString	mimetype = (*it)->name();
		m_mimelist << mimetype;
	}

	m_mimelist.sort();
	m_mimetype->insertStringList(m_mimelist);
	m_availablemime->insertStringList(m_mimelist);

	setMainWidget(topmain);
	setDetailsWidget(dummy);
}

void KXmlCommandDlg::setCommand(KXmlCommand *xmlCmd)
{
	setCaption(i18n("Command Edit for %1").arg(xmlCmd->name()));

	m_cmd = xmlCmd;
	m_description->setText(i18n(xmlCmd->description().utf8()));
	m_idname->setText(xmlCmd->name());

	m_requirements->clear();
	TQStringList	list = xmlCmd->requirements();
	TQListViewItem	*item(0);
	for (TQStringList::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		item = new TQListViewItem(m_requirements, item, *it);
		item->setRenameEnabled(0, true);
	}

	int	index = m_mimelist.findIndex(xmlCmd->mimeType());
	if (index != -1)
		m_mimetype->setCurrentItem(index);
	else
		m_mimetype->setCurrentItem(0);

	list = xmlCmd->inputMimeTypes();
	m_selectedmime->clear();
	m_availablemime->clear();
	m_availablemime->insertStringList(m_mimelist);
	for (TQStringList::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		m_selectedmime->insertItem(*it);
		delete m_availablemime->findItem(*it, TQt::ExactMatch);
	}
}

void KXmlCommandDlg::slotOk()
{
	if (m_cmd)
	{
		m_cmd->setMimeType((m_mimetype->currentText() == "all/all" ? TQString::null : m_mimetype->currentText()));
		m_cmd->setDescription(m_description->text());
		TQStringList	l;
		TQListViewItem	*item = m_requirements->firstChild();
		while (item)
		{
			l << item->text(0);
			item = item->nextSibling();
		}
		m_cmd->setRequirements(l);
		l.clear();
		for (uint i=0; i<m_selectedmime->count(); i++)
			l << m_selectedmime->text(i);
		m_cmd->setInputMimeTypes(l);
	}
	KDialogBase::slotOk();
}

bool KXmlCommandDlg::editCommand(KXmlCommand *xmlCmd, TQWidget *parent)
{
	if (!xmlCmd)
		return false;

	KXmlCommandDlg	xmldlg(parent, 0);
	xmldlg.setCommand(xmlCmd);

	return (xmldlg.exec() == Accepted);
}

void KXmlCommandDlg::slotAddMime()
{
	int	index = m_availablemime->currentItem();
	if (index != -1)
	{
		m_selectedmime->insertItem(m_availablemime->currentText());
		m_availablemime->removeItem(index);
		m_selectedmime->sort();
	}
}

void KXmlCommandDlg::slotRemoveMime()
{
	int	index = m_selectedmime->currentItem();
	if (index != -1)
	{
		m_availablemime->insertItem(m_selectedmime->currentText());
		m_selectedmime->removeItem(index);
		m_availablemime->sort();
	}
}

void KXmlCommandDlg::slotEditCommand()
{
	KXmlCommandAdvancedDlg::editCommand(m_cmd, parentWidget());
}

void KXmlCommandDlg::slotAddReq()
{
	TQListViewItem	*item = new TQListViewItem(m_requirements, m_requirements->lastItem(), i18n("exec:/"));
	item->setRenameEnabled(0, true);
	m_requirements->ensureItemVisible(item);
	item->startRename(0);
}

void KXmlCommandDlg::slotRemoveReq()
{
	delete m_requirements->currentItem();
}

void KXmlCommandDlg::slotReqSelected(TQListViewItem *item)
{
	m_removereq->setEnabled(item);
}

void KXmlCommandDlg::slotAvailableSelected(TQListBoxItem *item)
{
	m_addmime->setEnabled(item);
}

void KXmlCommandDlg::slotSelectedSelected(TQListBoxItem *item)
{
	m_removemime->setEnabled(item);
}

#include "kxmlcommanddlg.moc"
