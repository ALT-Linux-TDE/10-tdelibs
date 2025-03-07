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

/*
 *  "WhatsThis" help items added by Kurt Pfeifle, August 2003,
 *  same copyright as above.
 **/

#include "kprintdialog.h"
#include "kprinter.h"
#include "kprinterimpl.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kmmanager.h"
#include "kmprinter.h"
#include "kmvirtualmanager.h"
#include "kprintdialogpage.h"
#include "kprinterpropertydialog.h"
#include "plugincombobox.h"
#include "kpcopiespage.h"
#include "treecombobox.h"
#include "messagewindow.h"

#include <tqgroupbox.h>
#include <tqcheckbox.h>
#include <kpushbutton.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqtabwidget.h>
#include <tqvbox.h>
#include <tqlayout.h>
#include <tqregexp.h>
#include <tdemessagebox.h>
#include <tqdir.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <tdelocale.h>
#include <kiconloader.h>
#include <tdefiledialog.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kguiitem.h>
#include <kstdguiitem.h>
#include <tdeapplication.h>
#include <tdeio/renamedlg.h>

#include <time.h>

#define	SHOWHIDE(widget,on)	if (on) widget->show(); else widget->hide();

class KPrintDialog::KPrintDialogPrivate
{
public:
	TQLabel	*m_type, *m_state, *m_comment, *m_location, *m_cmdlabel, *m_filelabel;
	KPushButton	*m_properties, *m_default, *m_options, *m_ok, *m_extbtn;
	TQPushButton	*m_wizard, *m_filter;
	TQCheckBox	*m_preview;
	TQLineEdit	*m_cmd;
	TreeComboBox	*m_printers;
	TQVBox		*m_dummy;
	PluginComboBox	*m_plugin;
	KURLRequester	*m_file;
	TQCheckBox	*m_persistent;
	bool	m_reduced;

	TQPtrList<KPrintDialogPage>	m_pages;
	KPrinter		*m_printer;
	bool b_optionsEnabled;
	bool b_propertiesEnabled;
	bool b_systemEnabled;
};

KPrintDialog::KPrintDialog(TQWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	TQString whatsThisLocationLabel = i18n(  " <qt><b>Printer Location:</b> The <em>Location</em> may describe where the"
						" selected printer is located. The Location description is created"
						" by the administrator of the print system (or may be"
						" left empty)."
						" </qt>" );
	TQString whatsThisPrinterType = i18n(    " <qt><b>Printer Type:</b>  The <em>Type</em> indicates your printer type."
						" </qt>" );
	TQString whatsThisPrinterState = i18n(   " <qt><b>Printer State:</b>  The <em>State</em> indicates the state of the"
						" print queue on the print server (which could be your localhost). The"
						" state may be 'Idle', 'Processing', 'Stopped', 'Paused' or similar."
						" </qt>" );
	TQString whatsThisPrinterComment = i18n( " <qt><b>Printer Comment:</b>  The <em>Comment</em> may describe the selected"
						" printer. This comment is created by the administrator"
						" of the print system (or may be left empty)."
						" </qt>" );
	TQString whatsThisPrinterSelect = i18n(  " <qt><b>Printer Selection Menu:</b> "
						" <p>Use this combo box to select the printer to which you want to print."
						" Initially (if you run TDEPrint for the first time), you may only find the "
						"  <em>TDE special printers</em> (which save"
						" jobs to disk [as PostScript- or PDF-files], or deliver jobs via"
						" email (as a PDF"
						" attachment). If you are missing a real printer, you need to..."
						" <ul>"
						" <li>...either create a local printer with the help of the <em>TDE Add"
						" Printer Wizard</em>. The Wizard is available for the CUPS and RLPR printing"
						" systems (click button to the left of the <em>'Properties'</em> button),</li>"
						" <li>...or try to connect to an existing remote"
						" CUPS print server. You can connect by clicking the <em>'System Options'</em> button"
						" below. A new dialog opens: click on the <em>'CUPS server'</em>"
						" icon: Fill in the information required to use the remote"
						" server. </li> "
						" </ul>"
						" <p><b>Note:</b> It may happen that you successfully connected to a remote CUPS "
						" server and still do not get a printer list. If this happens: force TDEPrint to "
						" re-load its configuration files."
						" To reload the configuration files, either start kprinter again, or use the "
						" switch the print system away from CUPS and back again once. The print system switch "
						" can be made through a selection in the drop-down menu at bottom of this dialog when "
						" fully expanded). </p> "
						" </qt>" );
	TQString whatsThisPrintJobProperties = i18n( " <qt><b>Print Job Properties:</b> "
						" <p>This button opens a dialog where you can make decisions"
						" regarding all supported print job options."
						" </qt>" );
	TQString whatsThisPrinterFilter = i18n(  " <qt><b>Selective View on List of Printers:</b>"
						" <p> This button reduces the list of visible printers"
						" to a shorter, more convenient, pre-defined list.</p>"
						" <p>This is particularly useful in enterprise environments"
						" with lots of printers. The default is to show <b>all</b> printers.</p>"
						" <p>To create a personal <em>'selective view list'</em>, click on the"
						" <em>'System Options'</em> button at the bottom of this dialog."
						" Then, in the new dialog, select <em>'Filter'</em> (left column in the"
						" <em>TDE Print Configuration</em> dialog) and setup your selection..</p>"
						" <p><b>Warning:</b> Clicking this button without prior creation of a personal "
						" <em>'selective view list'</em> will make all printers dissappear from the "
						" view. (To re-enable all printers, just click this button again.) </p> "
						" </qt>" );
	TQString whatsThisAddPrinterWizard = i18n( "<qt><b>TDE Add Printer Wizard</b>"
						" <p>This button starts the <em>TDE Add Printer Wizard</em>.</p>"
						" <p>Use the Wizard (with <em>\"CUPS\"</em> or <em>\"RLPR\"</em>) to add locally"
						" defined printers to your system. </p>"
						" <p><b>Note:</b> The <em>TDE Add Printer Wizard</em> does <b>not</b> work, "
						" and this button is disabled if you use "
						" <em>\"Generic LPD</em>\", <em>\"LPRng\"</em>, or <em>\"Print Through "
						" an External Program</em>\".) </p> " 
						" </qt>" );
	TQString whatsThisExternalPrintCommand = i18n( " <qt><b>External Print Command</b>"
						" <p>Here you can enter any command that would also print for you in "
						" a <em>konsole</em> window. </p>"
						" <b>Example:</b> <pre>a2ps -P &lt;printername&gt; --medium=A3</pre>."
						" </qt>" );
	TQString whatsThisOptions = i18n( " <qt><b>Additional Print Job Options</b>"
						" <p>This button shows or hides additional printing options.</qt>" );
	TQString whatsThisSystemOptions = i18n(  " <qt><b>System Options:</b> "
						" <p>This button starts a new dialog where you can adjust various"
  						" settings of your printing system. Amongst them:"
						" <ul><li> Should \tDE"
						" applications embed all fonts into the PostScript they"
 						" generate for printing?"
						" <li> Should TDE use an external PostScript viewer"
						" like <em>gv</em> for print page previews?"
						" <li> Should TDEPrint use a local or a remote CUPS server?,"
						" </ul> and many more.... "
					        " </qt>" );

	TQString whatsThisHelpButton = i18n(     " <qt><b>Help:</b> This button takes you to the complete <em>TDEPrint"
						" Manual</em>."
					        " </qt>" );

	TQString whatsThisCancelButton = i18n(   " <qt><b>Cancel:</b> This button cancels your print job and quits the"
						" kprinter dialog."
					        " </qt>" );

	TQString whatsThisPrintButton = i18n(    " <qt><b>Print:</b> This button sends the job to the printing process."
						" If you are sending non-PostScript files, you may be"
						" asked if you want TDE to convert the files into PostScript,"
						" or if you want your print subsystem (like CUPS) to do this."
					        " </qt>" );

	TQString whatsThisKeepDialogOpenCheckbox = i18n( " <qt><b>Keep Printing Dialog Open</b>"
						"<p>If you enable this checkbox, the printing dialog"
						" stays open after you hit the <em>Print</em> button.</p>"
						" <p> This is"
						" especially useful, if you need to test various"
						" print settings (like color matching for an inkjet printer)"
						" or if you want to send your job to multiple printers (one after"
						" the other) to have it finished more quickly.</p>"
					        " </qt>" );

	TQString whatsThisOutputFileLabel = i18n(" <qt><b>Output File Name and Path:</b> The \"Output file:\" shows "
						" you where your file will be"
						" saved if you decide to \"Print to File\" your job, using one of the"
						" TDE <em>Special Printers</em> named \"Print to File (PostScript)\""
						" or \"Print to File (PDF)\". Choose a name and location that suits"
						" your need by using the button and/or editing the line on the right."
					        " </qt>" );

	TQString whatsThisOutputFileLineedit = i18n(" <qt><b>Output File Name and Path:</b> Edit this line to create a "
						" path and filename that suits your needs." 
						" (Button and Lineedit field are only available if you \"Print to File\")"
					        " </qt>" );

	TQString whatsThisOutputFileButton = i18n(" <qt><b>Browse Directories button:<b> This button calls "
						" the \"File Open / Browsed Directories\" dialog to let you"
						" choose a directory and file name where your \"Print-to-File\""
						" job should be saved."
					        " </qt>" );

	TQString whatsThisAddFileButton = i18n(  " <qt><b>Add File to Job</b>"
						" <p>This button calls the \"File Open / Browse Directories\" dialog to allow you"
						" to select a file for printing. Note, that "
						" <ul><li>you can select ASCII or International Text, PDF,"
						" PostScript, JPEG, TIFF, PNG, GIF and many other graphical"
						" formats."
						" <li>you can select various files from different paths"
						" and send them as one \"multi-file job\" to the printing"
						" system."
						" </ul>"
					        " </qt>" );

	TQString whatsThisPreviewCheckBox = i18n(" <qt><b>Print Preview</b>"
						" Enable this checkbox if you want to see a preview of"
						" your printout. A preview lets you check if, for instance,"
 						" your intended \"poster\" or \"pamphlet\" layout"
						" looks like you expected, without wasting paper first. It"
						" also lets you cancel the job if something looks wrong. "
						" <p><b>Note:</b> The preview feature (and therefore this checkbox) "
						" is only visible for printjobs created from inside TDE applications. "
						" If you start kprinter from the commandline, or if you use kprinter "
						" as a print command for non-TDE applications (like Acrobat Reader, "
                                                " Firefox or OpenOffice), print preview is not available here. "
					        " </qt>" );

	TQString whatsThisSetDefaultPrinter = i18n(" <qt><b>Set as Default Printer</b>"
						" This button sets the current printer as the user's"
						" default. "
						" <p><b>Note:</b> (Button is only visible if the checkbox for "
						" <em>System Options</em>"
						" --> <em>General</em> --> <em>Miscellaneous</em>: <em>\"Defaults"
						" to the last printer used in the application\"</em> is disabled.)"
					        " </qt>" );
	d = new KPrintDialogPrivate;

	d->m_pages.setAutoDelete(false);
	d->m_printer = 0;
	setCaption(i18n("Print"));

	// widget creation
	TQGroupBox	*m_pbox = new TQGroupBox(0,TQt::Vertical,i18n("Printer"), this);
	d->m_type = new TQLabel(m_pbox);
	TQWhatsThis::add(d->m_type, whatsThisPrinterType);
	d->m_state = new TQLabel(m_pbox);
	TQWhatsThis::add(d->m_state, whatsThisPrinterState);
	d->m_comment = new TQLabel(m_pbox);
	TQWhatsThis::add(d->m_comment, whatsThisPrinterComment);
	d->m_location = new TQLabel(m_pbox);
	TQWhatsThis::add(d->m_location, whatsThisLocationLabel);

	d->m_printers = new TreeComboBox(m_pbox);
	TQWhatsThis::add(d->m_printers, whatsThisPrinterSelect);
	d->m_printers->setMinimumHeight(25);
	TQLabel	*m_printerlabel = new TQLabel(i18n("&Name:"), m_pbox);
	TQWhatsThis::add(m_printerlabel, whatsThisPrinterSelect);
	TQLabel	*m_statelabel = new TQLabel(i18n("Status", "State:"), m_pbox);
	TQWhatsThis::add(m_statelabel, whatsThisPrinterState);
	TQLabel	*m_typelabel = new TQLabel(i18n("Type:"), m_pbox);
	TQWhatsThis::add(m_typelabel, whatsThisPrinterType);
	TQLabel	*m_locationlabel = new TQLabel(i18n("Location:"), m_pbox);
	TQWhatsThis::add(m_locationlabel, whatsThisLocationLabel);
	TQLabel	*m_commentlabel = new TQLabel(i18n("Comment:"), m_pbox);
	TQWhatsThis::add(m_commentlabel, whatsThisPrinterComment);
	m_printerlabel->setBuddy(d->m_printers);
	d->m_properties = new KPushButton(KGuiItem(i18n("P&roperties"), "edit"), m_pbox);
	TQWhatsThis::add( d->m_properties, whatsThisPrintJobProperties);
	d->m_options = new KPushButton(KGuiItem(i18n("System Op&tions"), "tdeprint_configmgr"), this);
	TQWhatsThis::add(d->m_options,whatsThisSystemOptions);
	d->m_default = new KPushButton(KGuiItem(i18n("Set as &Default"), "tdeprint_defaultsoft"), m_pbox);
	TQWhatsThis::add(d->m_default,whatsThisSetDefaultPrinter);
	d->m_filter = new TQPushButton(m_pbox);
	d->m_filter->setPixmap(SmallIcon("filter"));
	d->m_filter->setMinimumSize(TQSize(d->m_printers->minimumHeight(),d->m_printers->minimumHeight()));
	d->m_filter->setToggleButton(true);
	d->m_filter->setOn(KMManager::self()->isFilterEnabled());
	TQToolTip::add(d->m_filter, i18n("Toggle selective view on printer list"));
	TQWhatsThis::add(d->m_filter, whatsThisPrinterFilter);
	d->m_wizard = new TQPushButton(m_pbox);
	d->m_wizard->setPixmap(SmallIcon("wizard"));
	d->m_wizard->setMinimumSize(TQSize(d->m_printers->minimumHeight(),d->m_printers->minimumHeight()));
	TQToolTip::add(d->m_wizard, i18n("Add printer..."));
	TQWhatsThis::add(d->m_wizard, whatsThisAddPrinterWizard);
	d->m_ok = new KPushButton(KGuiItem(i18n("&Print"), "document-print"), this);
        TQWhatsThis::add( d->m_ok, whatsThisPrintButton);
	d->m_ok->setDefault(true);
	d->m_ok->setEnabled( false );
	TQPushButton	*m_cancel = new KPushButton(KStdGuiItem::cancel(), this);
        TQWhatsThis::add(m_cancel, whatsThisCancelButton);
	d->m_preview = new TQCheckBox(i18n("Previe&w"), m_pbox);
	TQWhatsThis::add(d->m_preview, whatsThisPreviewCheckBox);
	d->m_filelabel = new TQLabel(i18n("O&utput file:"), m_pbox);
	TQWhatsThis::add(d->m_filelabel,whatsThisOutputFileLabel);
	d->m_file = new KURLRequester(TQDir::homeDirPath()+"/print.ps", m_pbox);
	TQWhatsThis::add(d->m_file,whatsThisOutputFileLineedit);
	d->m_file->setEnabled(false);
	d->m_filelabel->setBuddy(d->m_file);
	d->m_cmdlabel = new TQLabel(i18n("Print co&mmand:"), m_pbox);
        TQWhatsThis::add( d->m_cmdlabel, whatsThisExternalPrintCommand);

	d->m_cmd = new TQLineEdit(m_pbox);
        TQWhatsThis::add( d->m_cmd, whatsThisExternalPrintCommand);
	d->m_cmdlabel->setBuddy(d->m_cmd);
	d->m_dummy = new TQVBox(this);
	d->m_plugin = new PluginComboBox(this);
	d->m_extbtn = new KPushButton(this);
	TQToolTip::add(d->m_extbtn, i18n("Show/hide advanced options"));
	TQWhatsThis::add(d->m_extbtn, whatsThisOptions);
	d->m_persistent = new TQCheckBox(i18n("&Keep this dialog open after printing"), this);
        TQWhatsThis::add( d->m_persistent, whatsThisKeepDialogOpenCheckbox);
	TQPushButton	*m_help = new KPushButton(KStdGuiItem::help(), this);
        TQWhatsThis::add( m_help, whatsThisHelpButton);

	TQWidget::setTabOrder( d->m_printers, d->m_filter );
	TQWidget::setTabOrder( d->m_filter, d->m_wizard );
	TQWidget::setTabOrder( d->m_wizard, d->m_properties );
	TQWidget::setTabOrder( d->m_properties, d->m_preview );
	TQWidget::setTabOrder( d->m_preview, d->m_file );
	TQWidget::setTabOrder( d->m_file, d->m_cmd );
	TQWidget::setTabOrder( d->m_plugin, d->m_persistent );
	TQWidget::setTabOrder( d->m_persistent, d->m_extbtn );
	TQWidget::setTabOrder( d->m_extbtn, d->m_options );
	TQWidget::setTabOrder( d->m_options, m_help );
	TQWidget::setTabOrder( m_help, d->m_ok );
	TQWidget::setTabOrder( d->m_ok, m_cancel );

	// layout creation
	TQVBoxLayout	*l1 = new TQVBoxLayout(this, 10, 10);
	l1->addWidget(m_pbox,0);
	l1->addWidget(d->m_dummy,1);
	l1->addWidget(d->m_plugin,0);
	l1->addWidget(d->m_persistent);
	TQHBoxLayout	*l2 = new TQHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addWidget(d->m_extbtn,0);
	l2->addWidget(d->m_options,0);
	l2->addWidget(m_help,0);
	l2->addStretch(1);
	l2->addWidget(d->m_ok,0);
	l2->addWidget(m_cancel,0);
	TQGridLayout	*l3 = new TQGridLayout(m_pbox->layout(),3,3,7);
	l3->setColStretch(1,1);
	l3->setRowStretch(0,1);
	TQGridLayout	*l4 = new TQGridLayout(0, 5, 2, 0, 5);
	l3->addMultiCellLayout(l4,0,0,0,1);
	l4->addWidget(m_printerlabel,0,0);
	l4->addWidget(m_statelabel,1,0);
	l4->addWidget(m_typelabel,2,0);
	l4->addWidget(m_locationlabel,3,0);
	l4->addWidget(m_commentlabel,4,0);
	TQHBoxLayout	*ll4 = new TQHBoxLayout(0, 0, 3);
	l4->addLayout(ll4,0,1);
	ll4->addWidget(d->m_printers,1);
	ll4->addWidget(d->m_filter,0);
	ll4->addWidget(d->m_wizard,0);
	//l4->addWidget(d->m_printers,0,1);
	l4->addWidget(d->m_state,1,1);
	l4->addWidget(d->m_type,2,1);
	l4->addWidget(d->m_location,3,1);
	l4->addWidget(d->m_comment,4,1);
	l4->setColStretch(1,1);
	TQVBoxLayout	*l5 = new TQVBoxLayout(0, 0, 10);
	l3->addLayout(l5,0,2);
	l5->addWidget(d->m_properties,0);
	l5->addWidget(d->m_default,0);
	l5->addWidget(d->m_preview,0);
	l5->addStretch(1);
	//***
	l3->addWidget(d->m_filelabel,1,0);
	l3->addWidget(d->m_file,1,1);
	//***
	l3->addWidget(d->m_cmdlabel,2,0);
	l3->addMultiCellWidget(d->m_cmd,2,2,1,2);

	// connections
	connect(d->m_ok,TQ_SIGNAL(clicked()),TQ_SLOT(accept()));
	connect(m_cancel,TQ_SIGNAL(clicked()),TQ_SLOT(reject()));
	connect(d->m_properties,TQ_SIGNAL(clicked()),TQ_SLOT(slotProperties()));
	connect(d->m_default,TQ_SIGNAL(clicked()),TQ_SLOT(slotSetDefault()));
	connect(d->m_printers,TQ_SIGNAL(activated(int)),TQ_SLOT(slotPrinterSelected(int)));
	connect(d->m_options,TQ_SIGNAL(clicked()),TQ_SLOT(slotOptions()));
	connect(d->m_wizard,TQ_SIGNAL(clicked()),TQ_SLOT(slotWizard()));
	connect(d->m_extbtn, TQ_SIGNAL(clicked()), TQ_SLOT(slotExtensionClicked()));
	connect(d->m_filter, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotToggleFilter(bool)));
	connect(m_help, TQ_SIGNAL(clicked()), TQ_SLOT(slotHelp()));
	connect(d->m_file, TQ_SIGNAL(urlSelected(const TQString&)), TQ_SLOT(slotOutputFileSelected(const TQString&)));
	connect( d->m_file, TQ_SIGNAL( openFileDialog( KURLRequester* ) ), TQ_SLOT( slotOpenFileDialog() ) );
	connect( KMFactory::self()->manager(), TQ_SIGNAL( updatePossible( bool ) ), TQ_SLOT( slotUpdatePossible( bool ) ) );

	d->b_optionsEnabled = kapp->authorize("print/options") && kapp->authorize("print/selection");
	d->b_propertiesEnabled = kapp->authorize("print/properties") && kapp->authorize("print/selection");
	d->b_systemEnabled = kapp->authorize("print/system") && kapp->authorize("print/selection");
	                
	if (!d->b_systemEnabled)
	{
		d->m_plugin->hide();
	}

	if (!d->b_optionsEnabled)
	{
		d->m_options->hide();
	}
	
	if (!d->b_propertiesEnabled)
	{
		d->m_properties->hide();
		d->m_wizard->hide();
	}

	if (!kapp->authorize("print/selection"))
	{
		d->m_extbtn->hide();
		m_pbox->hide();

		expandDialog(true);
	}
	else
	{
		TDEConfig	*config = TDEGlobal::config();
		config->setGroup("KPrinter Settings");
		expandDialog(!config->readBoolEntry("DialogReduced", (KMFactory::self()->settings()->application != KPrinter::StandAlone)));
	}
}

KPrintDialog::~KPrintDialog()
{
	TDEConfig	*config = TDEGlobal::config();
	config->setGroup("KPrinter Settings");
	config->writeEntry("DialogReduced", d->m_reduced);

	delete d;
}

void KPrintDialog::setFlags(int f)
{
	SHOWHIDE(d->m_properties, (f & KMUiManager::Properties) && d->b_propertiesEnabled)
	d->m_default->hide();
	SHOWHIDE(d->m_default, ((f & KMUiManager::Default) && !KMFactory::self()->printConfig("General")->readBoolEntry("UseLast", true)))
	SHOWHIDE(d->m_preview, (f & KMUiManager::Preview))
	bool	on = (f & KMUiManager::OutputToFile);
	SHOWHIDE(d->m_filelabel, on)
	SHOWHIDE(d->m_file, on)
	on = (f & KMUiManager::PrintCommand);
	SHOWHIDE(d->m_cmdlabel, on)
	SHOWHIDE(d->m_cmd, on)
	SHOWHIDE(d->m_persistent, (f & KMUiManager::Persistent))

	// also update "wizard" button
	KMManager	*mgr = KMFactory::self()->manager();
	d->m_wizard->setEnabled((mgr->hasManagement() && (mgr->printerOperationMask() & KMManager::PrinterCreation)));
}

void KPrintDialog::setDialogPages(TQPtrList<KPrintDialogPage> *pages)
{
	if (!pages) return;
	if (pages->count() + d->m_pages.count() == 1)
	{
		// only one page, reparent the page to d->m_dummy and remove any
		// TQTabWidget child if any.
		if (pages->count() > 0)
			d->m_pages.append(pages->take(0));
		d->m_pages.first()->reparent(d->m_dummy, TQPoint(0,0));
		d->m_pages.first()->show();
		delete d->m_dummy->child("TabWidget", "TQTabWidget");
	}
	else
	{
		// more than one page.
		TQTabWidget	*tabs = static_cast<TQTabWidget*>(d->m_dummy->child("TabWidget", "TQTabWidget"));
		if (!tabs)
		{
			// TQTabWidget doesn't exist. Create it and reparent all
			// already existing pages.
			tabs = new TQTabWidget(d->m_dummy, "TabWidget");
			tabs->setMargin(10);
			for (d->m_pages.first(); d->m_pages.current(); d->m_pages.next())
			{
				tabs->addTab(d->m_pages.current(), d->m_pages.current()->title());
			}
		}
		while (pages->count() > 0)
		{
			KPrintDialogPage	*page = pages->take(0);
			d->m_pages.append(page);
			tabs->addTab(page, page->title());
		}
		tabs->show();
	}
	d->m_extbtn->setEnabled(d->m_pages.count() > 0);
}

KPrintDialog* KPrintDialog::printerDialog(KPrinter *printer, TQWidget *parent, const TQString& caption, bool forceExpand)
{
	if (printer)
	{
		KPrintDialog	*dlg = new KPrintDialog(parent);
		// needs to set the printer before setting up the
		// print dialog as some additional pages may need it.
		// Real initialization comes after.
		dlg->d->m_printer = printer;
		KMFactory::self()->uiManager()->setupPrintDialog(dlg);
		dlg->init();
		if (!caption.isEmpty())
			dlg->setCaption(caption);
		if (forceExpand)
		{
			// we force the dialog to be expanded:
			//	- expand the dialog
			//	- hide the show/hide button
			dlg->expandDialog(true);
			dlg->d->m_extbtn->hide();
		}
		return dlg;
	}
	return NULL;
}

void KPrintDialog::initialize(KPrinter *printer)
{
	d->m_printer = printer;

	// first retrieve printer list and update combo box (get default or last used printer also)
	TQPtrList<KMPrinter>	*plist = KMFactory::self()->manager()->printerList();
	if (!KMManager::self()->errorMsg().isEmpty())
	{
		KMessageBox::error(parentWidget(),
			"<qt><nobr>"+
			i18n("An error occurred while retrieving the printer list:")
			+"</nobr><br><br>"+KMManager::self()->errorMsg()+"</qt>");
	}

	if (plist)
	{
		TQString	oldP = d->m_printers->currentText();
		d->m_printers->clear();
		TQPtrListIterator<KMPrinter>	it(*plist);
		int 	defsoft(-1), defhard(-1), defsearch(-1);
		bool	sep(false);
		for (;it.current();++it)
		{
			// skip invalid printers
			if ( !it.current()->isValid() )
				continue;

			if (!sep && it.current()->isSpecial())
			{
				sep = true;
				d->m_printers->insertItem(TQPixmap(), TQString::fromLatin1("--------"));
			}
			d->m_printers->insertItem(SmallIcon(it.current()->pixmap(),0,(it.current()->isValid() ? (int)TDEIcon::DefaultState : (int)TDEIcon::LockOverlay)),it.current()->name(),false/*sep*/);
			if (it.current()->isSoftDefault())
				defsoft = d->m_printers->count()-1;
			if (it.current()->isHardDefault())
				defhard = d->m_printers->count()-1;
			if (!oldP.isEmpty() && oldP == it.current()->name())
				defsearch = d->m_printers->count()-1;
			else if (defsearch == -1 && it.current()->name() == printer->searchName())
				defsearch = d->m_printers->count()-1;
		}
		int	defindex = (defsearch != -1 ? defsearch : (defsoft != -1 ? defsoft : TQMAX(defhard,0)));
		d->m_printers->setCurrentItem(defindex);
		//slotPrinterSelected(defindex);
	}

	// Initialize output filename
	if (!d->m_printer->outputFileName().isEmpty())
		d->m_file->setURL( d->m_printer->outputFileName() );
	else if (!d->m_printer->docFileName().isEmpty())
		d->m_file->setURL( d->m_printer->docDirectory()+"/"+d->m_printer->docFileName()+".ps" );

	if ( d->m_printers->count() > 0 )
		slotPrinterSelected( d->m_printers->currentItem() );

	// update with KPrinter options
	if (d->m_printer->option("kde-preview") == "1" || d->m_printer->previewOnly())
		d->m_preview->setChecked(true);
	d->m_preview->setEnabled(!d->m_printer->previewOnly());
	d->m_cmd->setText(d->m_printer->option("kde-printcommand"));
	TQPtrListIterator<KPrintDialogPage>	it(d->m_pages);
	for (;it.current();++it)
		it.current()->setOptions(d->m_printer->options());
}

void KPrintDialog::slotPrinterSelected(int index)
{
	bool 	ok(false);
	d->m_location->setText(TQString::null);
	d->m_state->setText(TQString::null);
	d->m_comment->setText(TQString::null);
	d->m_type->setText(TQString::null);
	if (index >= 0 && index < d->m_printers->count())
	{
		KMManager	*mgr = KMFactory::self()->manager();
		KMPrinter	*p = mgr->findPrinter(d->m_printers->text(index));
		if (p)
		{
			if (!p->isSpecial()) mgr->completePrinterShort(p);
			d->m_location->setText(p->location());
			d->m_comment->setText(p->driverInfo());
			d->m_type->setText(p->description());
			d->m_state->setText(p->stateString());
			ok = p->isValid();
			enableSpecial(p->isSpecial());
			enableOutputFile(p->option("kde-special-file") == "1");
			setOutputFileExtension(p->option("kde-special-extension"));
		}
                else
                    enableOutputFile( ok );
	}
	d->m_properties->setEnabled(ok);
	d->m_ok->setEnabled(ok);
}

void KPrintDialog::slotProperties()
{
	if (!d->m_printer) return;

	KMPrinter	*prt = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
	if (prt)
		KPrinterPropertyDialog::setupPrinter(prt, this);
}

void KPrintDialog::slotSetDefault()
{
	KMPrinter	*p = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
	if (p)
		KMFactory::self()->virtualManager()->setDefault(p);
}

void KPrintDialog::done(int result)
{
	if (result == Accepted && d->m_printer)
	{
		TQMap<TQString,TQString>	opts;
		KMPrinter		*prt(0);

		// get options from global pages
		TQString	msg;
		TQPtrListIterator<KPrintDialogPage>	it(d->m_pages);
		for (;it.current();++it)
			if (it.current()->isEnabled())
			{
				if (it.current()->isValid(msg))
					it.current()->getOptions(opts);
				else
				{
					KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"));
					return;
				}
			}

		// add options from the dialog itself
		// TODO: ADD PRINTER CHECK MECHANISM !!!
		prt = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
		if (prt->isSpecial() && prt->option("kde-special-file") == "1")
		{
			if (!checkOutputFile()) return;
			d->m_printer->setOutputToFile(true);
			/* be sure to decode the output filename */
			d->m_printer->setOutputFileName( KURL::decode_string( d->m_file->url() ) );
		}
		else
			d->m_printer->setOutputToFile(false);
		d->m_printer->setPrinterName(prt->printerName());
		d->m_printer->setSearchName(prt->name());
		opts["kde-printcommand"] = d->m_cmd->text();
		opts["kde-preview"] = (d->m_preview->isChecked() ? "1" : "0");
		opts["kde-isspecial"] = (prt->isSpecial() ? "1" : "0");
		opts["kde-special-command"] = prt->option("kde-special-command");

		// merge options with KMPrinter object options
		TQMap<TQString,TQString>	popts = (prt->isEdited() ? prt->editedOptions() : prt->defaultOptions());
		for (TQMap<TQString,TQString>::ConstIterator it=popts.begin(); it!=popts.end(); ++it)
			opts[it.key()] = it.data();

		// update KPrinter object
		d->m_printer->setOptions(opts);

		emit printRequested(d->m_printer);
		// close dialog if not persistent
		if (!d->m_persistent->isChecked() || !d->m_persistent->isVisible())
			KDialog::done(result);
	}
	else
		KDialog::done(result);
}

bool KPrintDialog::checkOutputFile()
{
	bool	value(false);
	if (d->m_file->url().isEmpty())
		KMessageBox::error(this,i18n("The output filename is empty."));
	else
	{
		KURL url( d->m_file->url() );
		if ( !url.isLocalFile() )
			return true;

		bool	anotherCheck;
		do
		{
		anotherCheck = false;
		TQFileInfo	f(url.path());
		if (f.exists())
		{
			if (f.isWritable())
			{
				//value = (KMessageBox::warningYesNo(this,i18n("File \"%1\" already exists. Overwrite?").arg(f.absFilePath())) == KMessageBox::Yes);
				time_t mtimeDest = f.lastModified().toTime_t();
				TDEIO::RenameDlg dlg( this, i18n( "Print" ), TQString::null, d->m_file->url(),
						TDEIO::M_OVERWRITE, ( time_t ) -1, f.size(), ( time_t ) -1, f.created().toTime_t() , mtimeDest+1, mtimeDest, true );
				int result = dlg.exec();
				switch ( result )
				{
					case TDEIO::R_OVERWRITE:
						value = true;
						break;
					default:
					case TDEIO::R_CANCEL:
						value = false;
						break;
					case TDEIO::R_RENAME:
						url = dlg.newDestURL();
						d->m_file->setURL( url.path() );
						value = true;
						anotherCheck = true;
						break;
				}
			}
			else
				KMessageBox::error(this,i18n("You don't have write permissions to this file."));
		}
		else
		{
			TQFileInfo d( f.dirPath( true ) );
			if ( !d.exists() )
				KMessageBox::error( this, i18n( "The output directory does not exist." ) );
			else if ( !d.isWritable() )
				KMessageBox::error(this,i18n("You don't have write permissions in that directory."));
			else
				value = true;
		}
		} while( anotherCheck );
	}
	return value;
}

void KPrintDialog::slotOptions()
{
	if (KMManager::self()->invokeOptionsDialog(this))
		init();
}

void KPrintDialog::enableOutputFile(bool on)
{
	d->m_filelabel->setEnabled(on);
	d->m_file->setEnabled(on);
}

void KPrintDialog::enableSpecial(bool on)
{
	d->m_default->setDisabled(on);
	d->m_cmdlabel->setDisabled(on);
	d->m_cmd->setDisabled(on);
	KPCopiesPage	*copypage = (KPCopiesPage*)child("CopiesPage","KPCopiesPage");
	if (copypage)
		copypage->initialize(!on);
	// disable/enable all other pages (if needed)
	for (d->m_pages.first(); d->m_pages.current(); d->m_pages.next())
		if (d->m_pages.current()->onlyRealPrinters())
			d->m_pages.current()->setEnabled(!on);
}

void KPrintDialog::setOutputFileExtension(const TQString& ext)
{
	if (!ext.isEmpty())
	{
		KURL url( d->m_file->url() );
		TQString f( url.fileName() );
		int p = f.findRev( '.' );
		// change "file.ext"; don't change "file", "file." or ".file" but do change ".file.ext"
		if ( p > 0 && p != int (f.length () - 1) )
		{
			url.setFileName( f.left( p ) + "." + ext );
			d->m_file->setURL( KURL::decode_string( url.url() ) );
		}
	}
}

void KPrintDialog::slotWizard()
{
	int	result = KMFactory::self()->manager()->addPrinterWizard(this);
	if (result == -1)
		KMessageBox::error(this, KMFactory::self()->manager()->errorMsg().prepend("<qt>").append("</qt>"));
	else if (result == 1)
		initialize(d->m_printer);
}

void KPrintDialog::reload()
{
	// remove printer dependent pages (usually from plugin)
	TQTabWidget	*tabs = static_cast<TQTabWidget*>(d->m_dummy->child("TabWidget", "TQTabWidget"));
	for (uint i=0; i<d->m_pages.count(); i++)
		if (d->m_pages.at(i)->onlyRealPrinters())
		{
			KPrintDialogPage	*page = d->m_pages.take(i--);
			if (tabs)
				tabs->removePage(page);
			delete page;
		}
	// reload printer dependent pages from plugin
	TQPtrList<KPrintDialogPage>	pages;
	pages.setAutoDelete(false);
	KMFactory::self()->uiManager()->setupPrintDialogPages(&pages);
	// add those pages to the dialog
	setDialogPages(&pages);
	if (!d->m_reduced)
		d->m_dummy->show();
	// other initializations
	setFlags(KMFactory::self()->uiManager()->dialogFlags());
	connect( KMFactory::self()->manager(), TQ_SIGNAL( updatePossible( bool ) ), TQ_SLOT( slotUpdatePossible( bool ) ) );
	init();
}

void KPrintDialog::configChanged()
{
	// simply update the printer list: do it all the time
	// as changing settings may influence the way printer
	// are listed.
	init();

	// update the GUI
	setFlags(KMFactory::self()->uiManager()->dialogFlags());
}

void KPrintDialog::expandDialog(bool on)
{
	TQSize	sz(size());
	bool	needResize(isVisible());

	if (on)
	{
		sz.setHeight(sz.height()+d->m_dummy->minimumSize().height()+d->m_plugin->minimumSize().height()+2*layout()->spacing());
		if (isVisible() || !d->m_dummy->isVisible() || !d->m_plugin->isVisible())
		{
			d->m_dummy->show();
			if (d->b_systemEnabled)
				d->m_plugin->show();
		}
		d->m_extbtn->setIconSet(SmallIconSet("go-up"));
		d->m_extbtn->setText(i18n("&Options <<"));
		d->m_reduced = false;
	}
	else
	{
		sz.setHeight(sz.height()-d->m_dummy->height()-d->m_plugin->height()-2*layout()->spacing());
		if (!isVisible() || d->m_dummy->isVisible() || d->m_plugin->isVisible())
		{
			d->m_dummy->hide();
			if (d->b_systemEnabled)
				d->m_plugin->hide();
		}
		d->m_extbtn->setIconSet(SmallIconSet("go-down"));
		d->m_extbtn->setText(i18n("&Options >>"));
		d->m_reduced = true;
	}

	if (needResize)
	{
		layout()->activate();
		resize(sz);
	}
}

void KPrintDialog::slotExtensionClicked()
{
	// As all pages are children of d->m_dummy, I simply have to hide/show it
	expandDialog(!(d->m_dummy->isVisible()));
}

KPrinter* KPrintDialog::printer() const
{
	return d->m_printer;
}

void KPrintDialog::slotToggleFilter(bool on)
{
	KMManager::self()->enableFilter(on);
	initialize(d->m_printer);
}

void KPrintDialog::slotHelp()
{
	kapp->invokeHelp(TQString::null, "tdeprint");
}

void KPrintDialog::slotOutputFileSelected(const TQString& txt)
{
	d->m_file->setURL( txt );
}

void KPrintDialog::init()
{
	d->m_ok->setEnabled( false );
	MessageWindow::remove( this );
	MessageWindow::add( this, i18n( "Initializing printing system..." ), 500 );
	KMFactory::self()->manager()->checkUpdatePossible();
}

void KPrintDialog::slotUpdatePossible( bool flag )
{
	MessageWindow::remove( this );
	if ( !flag )
		KMessageBox::error(parentWidget(),
			"<qt><nobr>"+
			i18n("An error occurred while retrieving the printer list:")
			+"</nobr><br><br>"+KMManager::self()->errorMsg()+"</qt>");
	initialize( d->m_printer );
}

void KPrintDialog::enableDialogPage( int index, bool flag )
{
	if ( index < 0 || index >= ( int )d->m_pages.count() )
	{
		kdWarning() << "KPrintDialog: page index out of bound" << endl;
		return;
	}

	if ( d->m_pages.count() > 1 )
	{
		TQTabWidget	*tabs = static_cast<TQTabWidget*>(d->m_dummy->child("TabWidget", "TQTabWidget"));
		tabs->setTabEnabled( d->m_pages.at( index ), flag );
	}
	else
		d->m_pages.at( 0 )->setEnabled( flag );
}

void KPrintDialog::slotOpenFileDialog()
{
	KFileDialog *dialog = d->m_file->fileDialog();

	dialog->setCaption(i18n("Print to File"));
	dialog->setMode(d->m_file->fileDialog()->mode() & ~KFile::LocalOnly);
	dialog->setOperationMode( KFileDialog::Saving );

	KMPrinter *prt = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
	if (prt)
	{
		TQString	mimetype(prt->option("kde-special-mimetype"));
		TQString	ext(prt->option("kde-special-extension"));

		if (!mimetype.isEmpty())
		{
			TQStringList filter;
			filter << mimetype;
			filter << "all/allfiles";
			dialog->setMimeFilter (filter, mimetype);
		}
		else if (!ext.isEmpty())
			dialog->setFilter ("*." + ext + "\n*|" + i18n ("All Files"));
	}
}

#include "kprintdialog.moc"
