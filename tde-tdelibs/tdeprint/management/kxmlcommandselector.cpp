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

#include "kxmlcommandselector.h"
#include "kxmlcommand.h"
#include "kxmlcommanddlg.h"
#include "tdeprintcheck.h"

#include <tqcombobox.h>
#include <kpushbutton.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqlineedit.h>
#include <kinputdialog.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <tdemessagebox.h>
#include <tdefiledialog.h>
#include <kseparator.h>
#include <kguiitem.h>
#include <kactivelabel.h>
#include <kdatetbl.h>
#include <kdialogbase.h>

KXmlCommandSelector::KXmlCommandSelector(bool canBeNull, TQWidget *parent, const char *name, KDialogBase *dlg)
: TQWidget(parent, name)
{
	m_cmd = new TQComboBox(this);
	connect(m_cmd, TQ_SIGNAL(activated(int)), TQ_SLOT(slotCommandSelected(int)));
	TQPushButton	*m_add = new KPushButton(this);
	TQPushButton	*m_edit = new KPushButton(this);
	m_add->setPixmap(SmallIcon("document-new"));
	m_edit->setPixmap(SmallIcon("configure"));
	connect(m_add, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddCommand()));
	connect(m_edit, TQ_SIGNAL(clicked()), TQ_SLOT(slotEditCommand()));
	TQToolTip::add(m_add, i18n("New command"));
	TQToolTip::add(m_edit, i18n("Edit command"));
	m_shortinfo = new TQLabel(this);
	m_helpbtn = new KPushButton( this );
	m_helpbtn->setIconSet( SmallIconSet( "help" ) );
	connect( m_helpbtn, TQ_SIGNAL( clicked() ), TQ_SLOT( slotHelpCommand() ) );
	TQToolTip::add( m_helpbtn, i18n( "Information" ) );
        m_helpbtn->setEnabled( false );

	m_line = 0;
	m_usefilter = 0;
	TQPushButton	*m_browse = 0;

	TQVBoxLayout	*l0 = new TQVBoxLayout(this, 0, 10);

	if (canBeNull)
	{
		m_line = new TQLineEdit(this);
		m_browse = new KPushButton(KGuiItem(i18n("&Browse..."), "document-open"), this);
		m_usefilter = new TQCheckBox(i18n("Use co&mmand:"), this);
		connect(m_browse, TQ_SIGNAL(clicked()), TQ_SLOT(slotBrowse()));
		connect(m_usefilter, TQ_SIGNAL(toggled(bool)), m_line, TQ_SLOT(setDisabled(bool)));
		connect(m_usefilter, TQ_SIGNAL(toggled(bool)), m_browse, TQ_SLOT(setDisabled(bool)));
		connect(m_usefilter, TQ_SIGNAL(toggled(bool)), m_cmd, TQ_SLOT(setEnabled(bool)));
		connect(m_usefilter, TQ_SIGNAL(toggled(bool)), m_add, TQ_SLOT(setEnabled(bool)));
		connect(m_usefilter, TQ_SIGNAL(toggled(bool)), m_edit, TQ_SLOT(setEnabled(bool)));
		connect(m_usefilter, TQ_SIGNAL(toggled(bool)), m_shortinfo, TQ_SLOT(setEnabled(bool)));
		connect( m_usefilter, TQ_SIGNAL( toggled( bool ) ), TQ_SLOT( slotXmlCommandToggled( bool ) ) );
		m_usefilter->setChecked(true);
		m_usefilter->setChecked(false);
		//setFocusProxy(m_line);
		setTabOrder(m_usefilter, m_cmd);
		setTabOrder(m_cmd, m_add);
		setTabOrder(m_add, m_edit);

		TQHBoxLayout	*l1 = new TQHBoxLayout(0, 0, 10);
		l0->addLayout(l1);
		l1->addWidget(m_line);
		l1->addWidget(m_browse);

		KSeparator	*sep = new KSeparator(TQt::Horizontal, this);
		l0->addWidget(sep);
	}
	else
		setFocusProxy(m_cmd);

	TQGridLayout	*l2 = new TQGridLayout(0, 2, (m_usefilter?3:2), 0, 5);
	int	c(0);
	l0->addLayout(l2);
	if (m_usefilter)
	{
		l2->addWidget(m_usefilter, 0, c++);
	}
	l2->addWidget(m_cmd, 0, c);
	TQHBoxLayout *l4 = new TQHBoxLayout( 0, 0, 5 );
	l2->addLayout( l4, 1, c );
	l4->addWidget( m_helpbtn, 0 );
	l4->addWidget( m_shortinfo, 1 );
	TQHBoxLayout	*l3 = new TQHBoxLayout(0, 0, 0);
	l2->addLayout(l3, 0, c+1);
	l3->addWidget(m_add);
	l3->addWidget(m_edit);

	if ( dlg )
		connect( this, TQ_SIGNAL( commandValid( bool ) ), dlg, TQ_SLOT( enableButtonOK( bool ) ) );

	loadCommands();
}

void KXmlCommandSelector::loadCommands()
{
	TQString	thisCmd = (m_cmd->currentItem() != -1 ? m_cmdlist[m_cmd->currentItem()] : TQString::null);

	m_cmd->clear();
	m_cmdlist.clear();

	TQStringList	list = KXmlCommandManager::self()->commandListWithDescription();
	TQStringList	desclist;
	for (TQStringList::Iterator it=list.begin(); it!=list.end(); ++it)
	{
		m_cmdlist << (*it);
		++it;
		desclist << (*it);
	}
	m_cmd->insertStringList(desclist);

	int	index = m_cmdlist.findIndex(thisCmd);
	if (index != -1)
		m_cmd->setCurrentItem(index);
	if (m_cmd->currentItem() != -1 && m_cmd->isEnabled())
		slotCommandSelected(m_cmd->currentItem());
}

TQString KXmlCommandSelector::command() const
{
	TQString	cmd;
	if (m_line && !m_usefilter->isChecked())
		cmd = m_line->text();
	else
		cmd = m_cmdlist[m_cmd->currentItem()];
	return cmd;
}

void KXmlCommandSelector::setCommand(const TQString& cmd)
{
	int	index = m_cmdlist.findIndex(cmd);

	if (m_usefilter)
		m_usefilter->setChecked(index != -1);
	if (m_line)
		m_line->setText((index == -1 ? cmd : TQString::null));
	if (index != -1)
		m_cmd->setCurrentItem(index);
	if (m_cmd->currentItem() != -1 && m_cmd->isEnabled())
		slotCommandSelected(m_cmd->currentItem());
}

void KXmlCommandSelector::slotAddCommand()
{
	bool	ok(false);
	TQString	cmdId = KInputDialog::getText(i18n("Command Name"), i18n("Enter an identification name for the new command:"), TQString::null, &ok, this);
	if (ok)
	{
		bool	added(true);

		if (m_cmdlist.findIndex(cmdId) != -1)
		{
			if (KMessageBox::warningContinueCancel(
				this,
				i18n("A command named %1 already exists. Do you want "
				     "to continue and edit the existing one?").arg(cmdId),
				TQString::null,
				KStdGuiItem::cont()) == KMessageBox::Cancel)
			{
				return;
			}
			else
				added = false;
	}

		KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(cmdId);
		if (KXmlCommandDlg::editCommand(xmlCmd, this))
			KXmlCommandManager::self()->saveCommand(xmlCmd);

		if (added)
			loadCommands();
	}
}

void KXmlCommandSelector::slotEditCommand()
{
	TQString	xmlId = m_cmdlist[m_cmd->currentItem()];
	KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(xmlId);
	if (xmlCmd)
	{
		if (KXmlCommandDlg::editCommand(xmlCmd, this))
		{
			// force to load the driver if not already done
			xmlCmd->driver();
			KXmlCommandManager::self()->saveCommand(xmlCmd);
		}
		m_cmd->changeItem(xmlCmd->description(), m_cmd->currentItem());
		delete xmlCmd;
		slotCommandSelected(m_cmd->currentItem());
	}
	else
		KMessageBox::error(this, i18n("Internal error. The XML driver for the command %1 could not be found.").arg(xmlId));
}

void KXmlCommandSelector::slotBrowse()
{
	TQString	filename = KFileDialog::getOpenFileName(TQString::null, TQString::null, this);
	if (!filename.isEmpty() && m_line)
		m_line->setText(filename);
}

void KXmlCommandSelector::slotCommandSelected(int ID)
{
	KXmlCommand	*xmlCmd = KXmlCommandManager::self()->loadCommand(m_cmdlist[ID], true);
	if (xmlCmd)
	{
		TQString msg;
		if ( xmlCmd->isValid() && KdeprintChecker::check( xmlCmd->requirements() ) )
		{
			msg = TQString::fromLocal8Bit("(ID = %1, %2 = ").arg(xmlCmd->name()).arg(i18n("output"));
			if (KXmlCommandManager::self()->checkCommand(xmlCmd->name(), KXmlCommandManager::None, KXmlCommandManager::Basic))
			{
				if (xmlCmd->mimeType() == "all/all")
					msg.append(i18n("undefined"));
				else
					msg.append(xmlCmd->mimeType());
			}
			else
				msg.append(i18n("not allowed"));
			msg.append(")");
			emit commandValid( true );
		}
		else
		{
			msg = "<font color=\"red\">" + i18n( "(Unavailable: requirements not satisfied)" ) + "</font>";
			emit commandValid( false );
		}
		m_shortinfo->setText(msg);
		m_help = xmlCmd->comment();
		m_helpbtn->setEnabled( !m_help.isEmpty() );
	}
	delete xmlCmd;
}

void KXmlCommandSelector::slotXmlCommandToggled( bool on )
{
	if ( on )
		slotCommandSelected( m_cmd->currentItem() );
	else
	{
		emit commandValid( true );
		m_shortinfo->setText( TQString::null );
	}
}

void KXmlCommandSelector::slotHelpCommand()
{
	TDEPopupFrame *pop = new TDEPopupFrame( m_helpbtn );
	KActiveLabel *lab = new KActiveLabel( m_help, pop );
	lab->resize( lab->sizeHint() );
	pop->setMainWidget( lab );
	pop->exec( m_helpbtn->mapToGlobal( TQPoint( m_helpbtn->width(), 0 ) ) );
	pop->close( 0 );
	delete pop;
}

#include "kxmlcommandselector.moc"
