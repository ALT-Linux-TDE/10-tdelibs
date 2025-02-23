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

#include "plugincombobox.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tdelocale.h>
#include <tqwhatsthis.h>

PluginComboBox::PluginComboBox(TQWidget *parent, const char *name)
:TQWidget(parent, name)
{
        TQString whatsThisCurrentPrintsystem = i18n(" <qt><b>Print Subsystem Selection</b>"
						" <p>This combo box shows (and lets you select)"
						" a print subsystem to be used by TDEPrint. (This print"
						" subsystem must, of course, be installed inside your"
						" Operating System.) TDEPrint usually auto-detects the" 
                                                " correct print subsystem by itself upon first startup."
						" Most Linux distributions have \"CUPS\", the <em>Common"
						" UNIX Printing System</em>." 
                                                " </qt>" );

	m_combo = new TQComboBox(this, "PluginCombo");
        TQWhatsThis::add(m_combo, whatsThisCurrentPrintsystem);
	TQLabel	*m_label = new TQLabel(i18n("Print s&ystem currently used:"), this);
        TQWhatsThis::add(m_label, whatsThisCurrentPrintsystem);
	m_label->setAlignment(AlignVCenter|AlignRight);
	m_label->setBuddy(m_combo);
	m_plugininfo = new TQLabel("Plugin information", this);
	TQGridLayout	*l0 = new TQGridLayout(this, 2, 2, 0, 5);
	l0->setColStretch(0, 1);
	l0->addWidget(m_label, 0, 0);
	l0->addWidget(m_combo, 0, 1);
	l0->addWidget(m_plugininfo, 1, 1);

	TQValueList<KMFactory::PluginInfo>	list = KMFactory::self()->pluginList();
	TQString			currentPlugin = KMFactory::self()->printSystem();
	for (TQValueList<KMFactory::PluginInfo>::ConstIterator it=list.begin(); it!=list.end(); ++it)
	{
		m_combo->insertItem((*it).comment);
		if ((*it).name == currentPlugin)
			m_combo->setCurrentItem(m_combo->count()-1);
		m_pluginlist.append((*it).name);
	}

	connect(m_combo, TQ_SIGNAL(activated(int)), TQ_SLOT(slotActivated(int)));
	configChanged();
}

void PluginComboBox::slotActivated(int index)
{
	TQString	plugin = m_pluginlist[index];
	if (!plugin.isEmpty())
	{
		// the factory will notify all registered objects of the change
		KMFactory::self()->reload(plugin, true);
	}
}

void PluginComboBox::reload()
{
	TQString	syst = KMFactory::self()->printSystem();
	int	index(-1);
	if ((index=m_pluginlist.findIndex(syst)) != -1)
		m_combo->setCurrentItem(index);
	configChanged();
}

void PluginComboBox::configChanged()
{
        TQString whatsThisCurrentConnection = i18n(" <qt><b>Current Connection</b>"
						" <p>This line shows which CUPS server your PC is"
						" currently connected to for printing and retrieving"
						" printer info. To switch to a different CUPS server,"
						" click \"System Options\", then select \"Cups server\""
						" and fill in the required info." 
                                                " </qt>" );

	m_plugininfo->setText(KMManager::self()->stateInformation());
        TQWhatsThis::add(m_plugininfo, whatsThisCurrentConnection);

}

#include "plugincombobox.moc"
