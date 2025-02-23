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

#include "kmcupsconfigwidget.h"
#include "cupsinfos.h"

#include <tqlabel.h>
#include <tqgroupbox.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqvalidator.h>

#include <tdelocale.h>
#include <kcursor.h>
#include <tdeconfig.h>
#include <kstringhandler.h>

class PortValidator : public TQIntValidator
{
public:
	PortValidator(TQWidget *parent, const char *name = 0);
	virtual TQValidator::State validate(TQString&, int&) const;
};

PortValidator::PortValidator(TQWidget *parent, const char *name)
: TQIntValidator(1, 65535, parent, name)
{
}

TQValidator::State PortValidator::validate(TQString& txt, int&) const
{
	bool 	ok(false);
	int 	p = txt.toInt(&ok);
	if (txt.isEmpty())
		return TQValidator::Intermediate;
	else if (ok && p >= bottom() && p <= top())
		return TQValidator::Acceptable;
	return TQValidator::Invalid;
}

//******************************************************************************************

KMCupsConfigWidget::KMCupsConfigWidget(TQWidget *parent, const char *name)
: TQWidget(parent,name)
{
	// widget creation
	TQGroupBox	*m_hostbox = new TQGroupBox(0, TQt::Vertical, i18n("Server Information"), this);
	TQGroupBox	*m_loginbox = new TQGroupBox(0, TQt::Vertical, i18n("Account Information"), this);
	TQLabel	*m_hostlabel = new TQLabel(i18n("&Host:"), m_hostbox);
	TQLabel	*m_portlabel = new TQLabel(i18n("&Port:"), m_hostbox);
	m_host = new TQLineEdit(m_hostbox);
	m_port = new TQLineEdit(m_hostbox);
	m_hostlabel->setBuddy(m_host);
	m_portlabel->setBuddy(m_port);
	m_port->setValidator(new PortValidator(m_port));
	m_login = new TQLineEdit(m_loginbox);
	TQLabel	*m_loginlabel = new TQLabel(i18n("&User:"), m_loginbox);
	TQLabel	*m_passwordlabel = new TQLabel(i18n("Pass&word:"), m_loginbox);
	m_password = new TQLineEdit(m_loginbox);
	m_password->setEchoMode(TQLineEdit::Password);
	m_savepwd = new TQCheckBox( i18n( "&Store password in configuration file" ), m_loginbox );
	m_savepwd->setCursor( KCursor::handCursor() );
	m_anonymous = new TQCheckBox(i18n("Use &anonymous access"), m_loginbox);
	m_anonymous->setCursor(KCursor::handCursor());
	m_loginlabel->setBuddy(m_login);
	m_passwordlabel->setBuddy(m_password);

	// layout creation
	TQVBoxLayout	*lay0 = new TQVBoxLayout(this, 0, 10);
	lay0->addWidget(m_hostbox,1);
	lay0->addWidget(m_loginbox,1);
	TQGridLayout	*lay2 = new TQGridLayout(m_hostbox->layout(), 2, 2, 10);
	lay2->setColStretch(1,1);
	lay2->addWidget(m_hostlabel,0,0);
	lay2->addWidget(m_portlabel,1,0);
	lay2->addWidget(m_host,0,1);
	lay2->addWidget(m_port,1,1);
	TQGridLayout	*lay3 = new TQGridLayout(m_loginbox->layout(), 4, 2, 10);
	lay3->setColStretch(1,1);
	lay3->addWidget(m_loginlabel,0,0);
	lay3->addWidget(m_passwordlabel,1,0);
	lay3->addWidget(m_login,0,1);
	lay3->addWidget(m_password,1,1);
	lay3->addMultiCellWidget(m_savepwd,2,2,0,1);
	lay3->addMultiCellWidget(m_anonymous,3,3,0,1);

	// connections
	connect(m_anonymous,TQ_SIGNAL(toggled(bool)),m_login,TQ_SLOT(setDisabled(bool)));
	connect(m_anonymous,TQ_SIGNAL(toggled(bool)),m_password,TQ_SLOT(setDisabled(bool)));
	connect(m_anonymous,TQ_SIGNAL(toggled(bool)),m_savepwd,TQ_SLOT(setDisabled(bool)));
}

void KMCupsConfigWidget::load()
{
	CupsInfos	*inf = CupsInfos::self();
	m_host->setText(inf->host());
	m_port->setText(TQString::number(inf->port()));
	if (inf->login().isEmpty())
		m_anonymous->setChecked(true);
	else
	{
		m_login->setText(inf->login());
		m_password->setText(inf->password());
		m_savepwd->setChecked( inf->savePassword() );
	}
}

void KMCupsConfigWidget::save(bool sync)
{
	CupsInfos	*inf = CupsInfos::self();
	inf->setHost(m_host->text());
	inf->setPort(m_port->text().toInt());
	if (m_anonymous->isChecked())
	{
		inf->setLogin(TQString::null);
		inf->setPassword(TQString::null);
		inf->setSavePassword( false );
	}
	else
	{
		inf->setLogin(m_login->text());
		inf->setPassword(m_password->text());
		inf->setSavePassword( m_savepwd->isChecked() );
	}
	if (sync) inf->save();
}

void KMCupsConfigWidget::saveConfig(TDEConfig *conf)
{
	conf->setGroup("CUPS");
	conf->writeEntry("Host",m_host->text());
	conf->writeEntry("Port",m_port->text().toInt());
	conf->writeEntry("Login",(m_anonymous->isChecked() ? TQString::null : m_login->text()));
	conf->writeEntry( "SavePassword", ( m_anonymous->isChecked() ? false : m_savepwd->isChecked() ) );
	if ( m_savepwd->isChecked() && !m_anonymous->isChecked() )
		conf->writeEntry( "Password", ( m_anonymous->isChecked() ? TQString::null : KStringHandler::obscure( m_password->text() ) ) );
	else
		conf->deleteEntry( "Password" );
	// synchronize CupsInfos object
	save(false);
}
