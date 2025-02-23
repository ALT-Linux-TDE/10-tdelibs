/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tdemainwindowiface.h"

#include <dcopclient.h>
#include <tdeapplication.h>
#include <kdcopactionproxy.h>
#include <kdcoppropertyproxy.h>
#include <tdemainwindow.h>
#include <tdeaction.h>
#include <tqclipboard.h>

#include <twin.h>

TDEMainWindowInterface::TDEMainWindowInterface(TDEMainWindow * mainWindow)
	: DCOPObject( mainWindow->name()) 
{
	m_MainWindow = mainWindow;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	m_dcopPropertyProxy = new KDCOPPropertyProxy(m_MainWindow);
}

TDEMainWindowInterface::~TDEMainWindowInterface()
{
	delete m_dcopActionProxy;
        delete m_dcopPropertyProxy;
}

QCStringList TDEMainWindowInterface::actions()
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	QCStringList tmp_actions;
	TQValueList<TDEAction *> lst = m_dcopActionProxy->actions();
	TQValueList<TDEAction *>::ConstIterator it = lst.begin();
	TQValueList<TDEAction *>::ConstIterator end = lst.end();
	for (; it != end; ++it )
		if ((*it)->isPlugged())
			tmp_actions.append( (TQCString)(*it)->name() );
	return tmp_actions;
}
bool TDEMainWindowInterface::activateAction( TQCString action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	TDEAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->activate();
		return true;
	}
	else
		return false;
}
bool TDEMainWindowInterface::disableAction( TQCString action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	TDEAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->setEnabled(false);
		return true;
	}
	else
		return false;
}
bool TDEMainWindowInterface::enableAction( TQCString action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	TDEAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		tmp_Action->setEnabled(true);
		return true;
	}
	else
		return false;
}
bool TDEMainWindowInterface::actionIsEnabled( TQCString action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	TDEAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		return tmp_Action->isEnabled();
	}
	else
		return false;
}
TQCString TDEMainWindowInterface::actionToolTip( TQCString action)
{
	delete m_dcopActionProxy;
	m_dcopActionProxy = new KDCOPActionProxy( m_MainWindow->actionCollection(), this );
	TDEAction *tmp_Action = m_dcopActionProxy->action(action);
	if (tmp_Action)
	{
		return tmp_Action->toolTip().utf8();
	}
	else
		return "Error no such object!";
}

DCOPRef TDEMainWindowInterface::action( const TQCString &name )
{
	return DCOPRef( kapp->dcopClient()->appId(), m_dcopActionProxy->actionObjectId( name ) );
}

TQMap<TQCString,DCOPRef> TDEMainWindowInterface::actionMap()
{
	return m_dcopActionProxy->actionMap();
}

int TDEMainWindowInterface::getWinID()
{
	return (int) m_MainWindow->winId();
}
void TDEMainWindowInterface::grabWindowToClipBoard()
{
	TQClipboard *clipboard = TQApplication::clipboard();
	clipboard->setPixmap(TQPixmap::grabWidget(m_MainWindow));
}
void TDEMainWindowInterface::hide()
{
	m_MainWindow->hide();
}
void TDEMainWindowInterface::maximize()
{
	m_MainWindow->showMaximized();
}
void TDEMainWindowInterface::minimize()
{
	m_MainWindow->showMinimized();
}
void TDEMainWindowInterface::resize(int newX, int newY)
{
	m_MainWindow->resize(newX, newY);
}
void TDEMainWindowInterface::move(int newX, int newY)
{
	m_MainWindow->move(newX, newY);
}
void TDEMainWindowInterface::setGeometry(int newX, int newY, int newWidth, int newHeight)
{
	m_MainWindow->setGeometry(newX, newY, newWidth, newHeight);
}
void TDEMainWindowInterface::raise()
{
	m_MainWindow->raise();
}
void TDEMainWindowInterface::lower()
{
	m_MainWindow->lower();
}
void TDEMainWindowInterface::restore()
{
	m_MainWindow->showNormal();
}
void TDEMainWindowInterface::close()
{
	m_MainWindow->close();
}
void TDEMainWindowInterface::show()
{
	m_MainWindow->show();
}
void TDEMainWindowInterface::setActiveWindow()
{
	m_MainWindow->setActiveWindow();
}
void TDEMainWindowInterface::setActiveWindowFocused()
{
	// just in case we don't have a WM running
	m_MainWindow->raise();
	m_MainWindow->setActiveWindow();

	// activate window (try to work around focus-stealing prevention)
	KWin::forceActiveWindow(m_MainWindow->winId());
}
QCStringList TDEMainWindowInterface::functionsDynamic()
{
	return m_dcopPropertyProxy->functions();
}
bool TDEMainWindowInterface::processDynamic(const TQCString &fun, const TQByteArray &data, TQCString& replyType, TQByteArray &replyData)
{
	return m_dcopPropertyProxy->processPropertyRequest( fun, data, replyType, replyData);

}

