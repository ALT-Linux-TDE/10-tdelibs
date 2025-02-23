/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <tdeglobal.h>
#include <kinstance.h>
#include <kiconloader.h>

#include "kmimesourcefactory.h"

class KMimeSourceFactoryPrivate
{
public:
  inline KMimeSourceFactoryPrivate (TDEIconLoader* loader) : m_iconLoader(loader), m_instance(0L) {}
  inline TDEIconLoader *iconLoader()
  {
    // If we don't have either of these, things are looking grim.
    Q_ASSERT(m_instance || m_iconLoader);

    if (m_iconLoader)
      return m_iconLoader;

    return m_instance->iconLoader();
  }

  TDEIconLoader *m_iconLoader;
  TDEInstance *m_instance;
};

KMimeSourceFactory::KMimeSourceFactory (TDEIconLoader* loader)
  : TQMimeSourceFactory (),
	d (new KMimeSourceFactoryPrivate (loader))
{
}

KMimeSourceFactory::~KMimeSourceFactory()
{
  delete d;
}

TQString KMimeSourceFactory::makeAbsolute (const TQString& absOrRelName, const TQString& context) const
{
  TQString myName;
  TQString myContext;

  const int pos = absOrRelName.find ('|');
  if (pos > -1)
	{
	  myContext = absOrRelName.left (pos);
	  myName = absOrRelName.right (absOrRelName.length() - myContext.length() - 1);
	}

  TQString result;

  if (myContext == "desktop")
	{
	  result = d->iconLoader()->iconPath (myName, TDEIcon::Desktop);
	}
  else if (myContext == "toolbar")
	{	 
	  result = d->iconLoader()->iconPath (myName, TDEIcon::Toolbar);
	}
  else if (myContext == "maintoolbar")
	{
	  result = d->iconLoader()->iconPath (myName, TDEIcon::MainToolbar);
	}
  else if (myContext == "small")
	{
	  result = d->iconLoader()->iconPath (myName, TDEIcon::Small);
	}
  else if (myContext == "user")
	{	  
	  result = d->iconLoader()->iconPath (myName, TDEIcon::User);
	}

  if (result.isEmpty())
	result = TQMimeSourceFactory::makeAbsolute (absOrRelName, context);
  
  return result;
}

void KMimeSourceFactory::setInstance(TDEInstance *instance)
{
  d->m_instance = instance;
}

void KMimeSourceFactory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

