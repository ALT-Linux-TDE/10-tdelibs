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

#include "kmdriverdbwidget.h"
#include "kmdriverdb.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "driver.h"

#include <tdelistbox.h>
#include <kpushbutton.h>
#include <tqcheckbox.h>
#include <kcursor.h>
#include <tqapplication.h>
#include <tdemessagebox.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqstrlist.h>

#include <tdelocale.h>
#include <kcursor.h>
#include <tdefiledialog.h>
#include <kguiitem.h>
#include <tdeio/netaccess.h>

KMDriverDbWidget::KMDriverDbWidget(TQWidget *parent, const char *name)
: TQWidget(parent,name)
{
	m_external = TQString::null;
	m_valid = false;

	// build widget
	m_manu = new TDEListBox(this);
	m_model = new TDEListBox(this);
	m_postscript = new TQCheckBox(i18n("&PostScript printer"),this);
	m_raw = new TQCheckBox(i18n("&Raw printer (no driver needed)"),this);
	m_postscript->setCursor(KCursor::handCursor());
	m_raw->setCursor(KCursor::handCursor());
	m_other = new KPushButton(KGuiItem(i18n("&Other..."), "document-open"), this);
	TQLabel	*l1 = new TQLabel(i18n("&Manufacturer:"), this);
	TQLabel	*l2 = new TQLabel(i18n("Mo&del:"), this);
	l1->setBuddy(m_manu);
	l2->setBuddy(m_model);

	// build layout
	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 0, 10);
	TQGridLayout	*sub1_ = new TQGridLayout(0, 2, 3, 0, 0);
	TQHBoxLayout	*sub2_ = new TQHBoxLayout(0, 0, 10);
	main_->addLayout(sub1_);
	main_->addLayout(sub2_);
	main_->addWidget(m_raw);
	sub1_->addWidget(l1,0,0);
	sub1_->addWidget(l2,0,2);
	sub1_->addWidget(m_manu,1,0);
	sub1_->addWidget(m_model,1,2);
	sub1_->addColSpacing(1,20);
	sub2_->addWidget(m_postscript,1);
	sub2_->addWidget(m_other,0);

	// build connections
	connect(KMDriverDB::self(),TQ_SIGNAL(dbLoaded(bool)),TQ_SLOT(slotDbLoaded(bool)));
	connect(KMDriverDB::self(), TQ_SIGNAL(error(const TQString&)), TQ_SLOT(slotError(const TQString&)));
	connect(m_manu,TQ_SIGNAL(highlighted(const TQString&)),TQ_SLOT(slotManufacturerSelected(const TQString&)));
	connect(m_raw,TQ_SIGNAL(toggled(bool)),m_manu,TQ_SLOT(setDisabled(bool)));
	connect(m_raw,TQ_SIGNAL(toggled(bool)),m_model,TQ_SLOT(setDisabled(bool)));
	connect(m_raw,TQ_SIGNAL(toggled(bool)),m_other,TQ_SLOT(setDisabled(bool)));
	connect(m_raw,TQ_SIGNAL(toggled(bool)),m_postscript,TQ_SLOT(setDisabled(bool)));
	connect(m_postscript,TQ_SIGNAL(toggled(bool)),m_manu,TQ_SLOT(setDisabled(bool)));
	connect(m_postscript,TQ_SIGNAL(toggled(bool)),m_model,TQ_SLOT(setDisabled(bool)));
	connect(m_postscript,TQ_SIGNAL(toggled(bool)),m_other,TQ_SLOT(setDisabled(bool)));
	connect(m_postscript,TQ_SIGNAL(toggled(bool)),m_raw,TQ_SLOT(setDisabled(bool)));
	connect(m_postscript,TQ_SIGNAL(toggled(bool)),TQ_SLOT(slotPostscriptToggled(bool)));
	connect(m_other,TQ_SIGNAL(clicked()),TQ_SLOT(slotOtherClicked()));
}

KMDriverDbWidget::~KMDriverDbWidget()
{
}

void KMDriverDbWidget::setDriver(const TQString& manu, const TQString& model)
{
	TQListBoxItem	*item = m_manu->findItem(manu);
	TQString		model_(model);
	if (item)
	{
		m_manu->setCurrentItem(item);
		item = m_model->findItem(model_);
		if (!item)
			// try by stripping the manufacturer name from
			// the beginning of the model string. This is
			// often the case with PPD files
			item = m_model->findItem(model_.replace(0,manu.length()+1,TQString::fromLatin1("")));
		if (item)
			m_model->setCurrentItem(item);
	}
}

void KMDriverDbWidget::setHaveRaw(bool on)
{
	if (on)
		m_raw->show();
	else
		m_raw->hide();
}

void KMDriverDbWidget::setHaveOther(bool on)
{
	if (on)
		m_other->show();
	else
		m_other->hide();
}

TQString KMDriverDbWidget::manufacturer()
{
	return m_manu->currentText();
}

TQString KMDriverDbWidget::model()
{
	return m_model->currentText();
}

KMDBEntryList* KMDriverDbWidget::drivers()
{
	return KMDriverDB::self()->findEntry(manufacturer(),model());
}

bool KMDriverDbWidget::isRaw()
{
	return m_raw->isChecked();
}

void KMDriverDbWidget::init()
{
	if (!m_valid)
	{
		TQApplication::setOverrideCursor(KCursor::waitCursor());
		m_manu->clear();
		m_model->clear();
		m_manu->insertItem(i18n("Loading..."));
		KMDriverDB::self()->init(this);
	}
}

void KMDriverDbWidget::slotDbLoaded(bool reloaded)
{
	TQApplication::restoreOverrideCursor();
	m_valid = true;
	if (reloaded || m_manu->count() == 0 || (m_manu->count() == 1 && m_manu->text(0) == i18n("Loading...")))
	{ // do something only if DB reloaded
		m_manu->clear();
		m_model->clear();
		TQDictIterator< TQDict<KMDBEntryList> >	it(KMDriverDB::self()->manufacturers());
		for (;it.current();++it)
			m_manu->insertItem(it.currentKey());
		m_manu->sort();
		m_manu->setCurrentItem(0);
	}
}

void KMDriverDbWidget::slotError(const TQString& msg)
{
	TQApplication::restoreOverrideCursor();
	m_valid = false;
	m_manu->clear();
	KMessageBox::error(this, "<qt>"+msg+"</qt>");
}

void KMDriverDbWidget::slotManufacturerSelected(const TQString& name)
{
	m_model->clear();
	TQDict<KMDBEntryList>	*models = KMDriverDB::self()->findModels(name);
	if (models)
	{
		TQStrIList	ilist(true);
		TQDictIterator<KMDBEntryList>	it(*models);
		for (;it.current();++it)
			ilist.append(it.currentKey().latin1());
		ilist.sort();
		m_model->insertStrList(&ilist);
		m_model->setCurrentItem(0);
	}
}

void KMDriverDbWidget::slotPostscriptToggled(bool on)
{
	if (on)
	{
		TQListBoxItem	*item = m_manu->findItem("GENERIC");
		if (item)
		{
			m_manu->setCurrentItem(item);
			item = m_model->findItem( "POSTSCRIPT PRINTER" );
			if ( item )
			{
				m_model->setCurrentItem( item );
				return;
			}
		}
		KMessageBox::error(this,i18n("Unable to find the PostScript driver."));
		m_postscript->setChecked(false);
	}
}

void KMDriverDbWidget::slotOtherClicked()
{
	if (m_external.isEmpty())
	{
		KFileDialog dlg( TQString::null, TQString::null, this, 0, true );
		KURL url;

		dlg.setMode( KFile::File );
		dlg.setCaption( i18n( "Select Driver" ) );
		if ( dlg.exec() )
			url = dlg.selectedURL();

		if ( !url.isEmpty() )
		{
			TQString filename;
			if ( TDEIO::NetAccess::download( url, filename, this ) )
			{
				DrMain	*driver = KMFactory::self()->manager()->loadFileDriver(filename);
				if (driver)
				{
					m_external = filename;
					disconnect(m_manu,TQ_SIGNAL(highlighted(const TQString&)),this,TQ_SLOT(slotManufacturerSelected(const TQString&)));
					m_manu->clear();
					m_model->clear();
					TQString	s = driver->get("manufacturer");
					m_manu->insertItem((s.isEmpty() ? i18n("<Unknown>") : s));
					s = driver->get("model");
					m_model->insertItem((s.isEmpty() ? i18n("<Unknown>") : s));
					m_manu->setCurrentItem(0);
					m_model->setCurrentItem(0);
					m_other->setText(i18n("Database"));
					m_desc = driver->get("description");
					delete driver;
				}
				else
				{
					TDEIO::NetAccess::removeTempFile( filename );
					KMessageBox::error(this,"<qt>"+i18n("Wrong driver format.")+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");
				}
			}
		}
	}
	else
	{
		m_external = TQString::null;
		connect(m_manu,TQ_SIGNAL(highlighted(const TQString&)),this,TQ_SLOT(slotManufacturerSelected(const TQString&)));
		m_other->setText(i18n("Other"));
		m_desc = TQString::null;
		slotDbLoaded(true);
	}
}
#include "kmdriverdbwidget.moc"
