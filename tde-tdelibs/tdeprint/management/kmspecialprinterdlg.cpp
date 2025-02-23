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

#include "kmspecialprinterdlg.h"
#include "kmprinter.h"
#include "tdeprintcheck.h"
#include "kmfactory.h"
#include "kmspecialmanager.h"
#include "kxmlcommandselector.h"
#include "kxmlcommand.h"
#include "driver.h"

#include <tqpushbutton.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqgroupbox.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kicondialog.h>
#include <tdefiledialog.h>
#include <kseparator.h>

KMSpecialPrinterDlg::KMSpecialPrinterDlg(TQWidget *parent, const char *name)
: KDialogBase(parent, name, true, TQString::null, Ok|Cancel, Ok)
{
	setCaption(i18n("Add Special Printer"));

	TQWidget	*dummy = new TQWidget(this);
	setMainWidget(dummy);

	// widget creation
	m_name = new TQLineEdit(dummy);
	connect(m_name, TQ_SIGNAL(textChanged ( const TQString & )),this,TQ_SLOT(slotTextChanged(const TQString & )));
	m_description = new TQLineEdit(dummy);
	m_location = new TQLineEdit(dummy);
	TQLabel	*m_namelabel = new TQLabel(i18n("&Name:"), dummy);
	TQLabel	*m_desclabel = new TQLabel(i18n("&Description:"), dummy);
	TQLabel	*m_loclabel = new TQLabel(i18n("&Location:"), dummy);
	m_namelabel->setBuddy(m_name);
	m_desclabel->setBuddy(m_description);
	m_loclabel->setBuddy(m_location);

	KSeparator* sep = new KSeparator( KSeparator::HLine, dummy);

	sep->setFixedHeight(10);
	TQGroupBox	*m_gb = new TQGroupBox(1, TQt::Horizontal, i18n("Command &Settings"), dummy);
	m_command = new KXmlCommandSelector(true, m_gb, "CommandSelector", this);

	TQGroupBox *m_outfile_gb = new TQGroupBox( 0, TQt::Horizontal, i18n( "Outp&ut File" ), dummy );

	m_usefile = new TQCheckBox( i18n("&Enable output file"), m_outfile_gb);

	m_mimetype = new TQComboBox(m_outfile_gb);
	KMimeType::List	list = KMimeType::allMimeTypes();
	for (TQValueList<KMimeType::Ptr>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		TQString	mimetype = (*it)->name();
		m_mimelist << mimetype;
	}
	m_mimelist.sort();
	m_mimetype->insertStringList(m_mimelist);

	TQLabel	*m_mimetypelabel = new TQLabel(i18n("&Format:"), m_outfile_gb);
	m_mimetypelabel->setBuddy (m_mimetype);

	m_extension = new TQLineEdit(m_outfile_gb);

	TQLabel	*m_extensionlabel = new TQLabel(i18n("Filename e&xtension:"), m_outfile_gb);
	m_extensionlabel->setBuddy(m_extension);

	m_icon = new TDEIconButton(dummy);
	m_icon->setIcon("document-print");
	m_icon->setFixedSize(TQSize(48,48));

	connect( m_usefile, TQ_SIGNAL( toggled( bool ) ), m_mimetype, TQ_SLOT( setEnabled( bool ) ) );
	connect( m_usefile, TQ_SIGNAL( toggled( bool ) ), m_extension, TQ_SLOT( setEnabled( bool ) ) );
	connect( m_usefile, TQ_SIGNAL( toggled( bool ) ), m_mimetypelabel, TQ_SLOT( setEnabled( bool ) ) );
	connect( m_usefile, TQ_SIGNAL( toggled( bool ) ), m_extensionlabel, TQ_SLOT( setEnabled( bool ) ) );
	m_mimetypelabel->setEnabled( false );
	m_mimetype->setEnabled( false );
	m_extensionlabel->setEnabled( false );
	m_extension->setEnabled( false );

	TQWhatsThis::add(m_usefile,
		i18n("<p>The command will use an output file. If checked, make sure the "
		     "command contains an output tag.</p>"));
	TQWhatsThis::add(m_command,
		i18n("<p>The command to execute when printing on this special printer. Either enter "
			 "the command to execute directly, or associate/create a command object with/for "
			 "this special printer. The command object is the preferred method as it provides "
			 "support for advanced settings like mime type checking, configurable options and "
			 "requirement list (the plain command is only provided for backward compatibility). "
			 "When using a plain command, the following tags are recognized:</p>"
			 "<ul><li><b>%in</b>: the input file (required).</li>"
			 "<li><b>%out</b>: the output file (required if using an output file).</li>"
			 "<li><b>%psl</b>: the paper size in lower case.</li>"
			 "<li><b>%psu</b>: the paper size with the first letter in upper case.</li></ul>"));
	TQString mimetypeWhatsThis = i18n("<p>The default mimetype for the output file (e.g. application/postscript).</p>");
	TQWhatsThis::add(m_mimetypelabel, mimetypeWhatsThis);
	TQWhatsThis::add(m_mimetype, mimetypeWhatsThis);
	TQString extensionWhatsThis = i18n("<p>The default extension for the output file (e.g. ps, pdf, ps.gz).</p>");
	TQWhatsThis::add(m_extensionlabel, extensionWhatsThis);
	TQWhatsThis::add(m_extension, extensionWhatsThis);

	// layout creation
	TQVBoxLayout	*l0 = new TQVBoxLayout(dummy, 0, 10);
	TQGridLayout	*l1 = new TQGridLayout(0, 3, 3, 0, 5);
	l0->addLayout(l1);
	l1->setColStretch(2,1);
	l1->addColSpacing(0,60);
	l1->addMultiCellWidget(m_icon, 0, 2, 0, 0, TQt::AlignCenter);
	l1->addWidget(m_namelabel, 0, 1);
	l1->addWidget(m_desclabel, 1, 1);
	l1->addWidget(m_loclabel, 2, 1);
	l1->addWidget(m_name, 0, 2);
	l1->addWidget(m_description, 1, 2);
	l1->addWidget(m_location, 2, 2);
	l0->addWidget(sep);
	l0->addWidget(m_gb);
	l0->addWidget(m_outfile_gb);
	TQGridLayout	*l6 = new TQGridLayout(m_outfile_gb->layout(), 3, 2, 10);
	l6->addMultiCellWidget( m_usefile, 0, 0, 0, 1 );
	l6->addWidget(m_mimetypelabel, 1, 0);
	l6->addWidget(m_mimetype, 1, 1);
	l6->addWidget(m_extensionlabel, 2, 0);
	l6->addWidget(m_extension, 2, 1);

	enableButton(Ok, !m_name->text().isEmpty());

	// resize dialog
	resize(400,100);
}

void KMSpecialPrinterDlg::slotTextChanged(const TQString & )
{
	enableButton(Ok, !m_name->text().isEmpty());
}

void KMSpecialPrinterDlg::slotOk()
{
	if (!checkSettings())
		return;
	KDialogBase::slotOk();
}

bool KMSpecialPrinterDlg::checkSettings()
{
	TQString	msg;
	if (m_name->text().isEmpty())
		msg = i18n("You must provide a non-empty name.");
	else
		KXmlCommandManager::self()->checkCommand(m_command->command(),
					KXmlCommandManager::Basic,
					(m_usefile->isChecked() ? KXmlCommandManager::Basic : KXmlCommandManager::None),
					&msg);

	if (!msg.isEmpty())
		KMessageBox::error(this, i18n("Invalid settings. %1.").arg(msg));

	return (msg.isEmpty());
}

void KMSpecialPrinterDlg::setPrinter(KMPrinter *printer)
{
	if (printer && printer->isSpecial())
	{
		m_command->setCommand(printer->option("kde-special-command"));
		m_usefile->setChecked(printer->option("kde-special-file") == "1");
		int	index = m_mimelist.findIndex(printer->option("kde-special-mimetype"));
		m_mimetype->setCurrentItem(index == -1 ? 0 : index);
		m_extension->setText(printer->option("kde-special-extension"));
		m_name->setText(printer->name());
		m_description->setText(printer->description());
		m_location->setText(printer->location());
		m_icon->setIcon(printer->pixmap());

		setCaption(i18n("Configuring %1").arg(printer->name()));
	}
}

KMPrinter* KMSpecialPrinterDlg::printer()
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(m_name->text());
	printer->setPrinterName(m_name->text());
	printer->setPixmap(m_icon->icon());
	printer->setDescription(m_description->text());
	printer->setLocation(m_location->text());
	printer->setOption("kde-special-command",m_command->command());
	printer->setOption("kde-special-file",(m_usefile->isChecked() ? "1" : "0"));
	if (m_usefile->isChecked ())
	{
		if (m_mimetype->currentText() != "all/all")
			printer->setOption("kde-special-mimetype", m_mimetype->currentText());
		printer->setOption("kde-special-extension",m_extension->text());
	}
	printer->setType(KMPrinter::Special);
	printer->setState(KMPrinter::Idle);
	return printer;
}

#include "kmspecialprinterdlg.moc"
