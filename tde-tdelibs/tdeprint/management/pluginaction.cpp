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

#include "pluginaction.h"

// automatically connect to "pluginActionActived(int)" in the receiver.
PluginAction::PluginAction(int ID, const TQString& txt, const TQString& icon, int accel, TQObject *parent, const char *name)
: TDEAction(txt, icon, accel, parent, name), m_id(ID)
{
	connect(this, TQ_SIGNAL(activated()), TQ_SLOT(slotActivated()));
}

void PluginAction::slotActivated()
{
	emit activated(m_id);
}

#include "pluginaction.moc"
