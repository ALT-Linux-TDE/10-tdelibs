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

#include "kmproxywidget.h"

#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqvalidator.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kcursor.h>

KMProxyWidget::KMProxyWidget(TQWidget *parent, const char *name)
: TQGroupBox(0, TQt::Vertical, i18n("Proxy Settings"), parent, name)
{
	TQLabel	*m_hostlabel = new TQLabel(i18n("&Host:"), this);
	TQLabel	*m_portlabel = new TQLabel(i18n("&Port:"), this);
	m_useproxy = new TQCheckBox(i18n("&Use proxy server"), this);
	m_useproxy->setCursor(KCursor::handCursor());
	m_proxyhost = new TQLineEdit(this);
	m_proxyport = new TQLineEdit(this);
	m_proxyport->setValidator(new TQIntValidator(m_proxyport));
	m_hostlabel->setBuddy(m_proxyhost);
	m_portlabel->setBuddy(m_proxyport);
	
	connect(m_useproxy,TQ_SIGNAL(toggled(bool)),m_proxyhost,TQ_SLOT(setEnabled(bool)));
	connect(m_useproxy,TQ_SIGNAL(toggled(bool)),m_proxyport,TQ_SLOT(setEnabled(bool)));
	m_proxyhost->setEnabled(false);
	m_proxyport->setEnabled(false);

	TQGridLayout	*lay0 = new TQGridLayout(layout(), 3, 2, 10);
	lay0->setColStretch(1,1);
	lay0->addMultiCellWidget(m_useproxy,0,0,0,1);
	lay0->addWidget(m_hostlabel,1,0);
	lay0->addWidget(m_portlabel,2,0);
	lay0->addWidget(m_proxyhost,1,1);
	lay0->addWidget(m_proxyport,2,1);
}

void KMProxyWidget::loadConfig(TDEConfig *conf)
{
	conf->setGroup("RLPR");
	m_proxyhost->setText(conf->readEntry("ProxyHost",TQString::null));
	m_proxyport->setText(conf->readEntry("ProxyPort",TQString::null));
	m_useproxy->setChecked(!m_proxyhost->text().isEmpty());
}

void KMProxyWidget::saveConfig(TDEConfig *conf)
{
	conf->setGroup("RLPR");
	conf->writeEntry("ProxyHost",(m_useproxy->isChecked() ? m_proxyhost->text() : TQString::null));
	conf->writeEntry("ProxyPort",(m_useproxy->isChecked() ? m_proxyport->text() : TQString::null));
}
