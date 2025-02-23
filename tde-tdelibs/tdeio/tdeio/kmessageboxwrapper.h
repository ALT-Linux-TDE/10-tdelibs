/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#ifndef KMESSAGEBOXWRAPPER_H
#define KMESSAGEBOXWRAPPER_H
#include <tdemessagebox.h>
#include <tdeapplication.h>
#include <kdebug.h>

/**
 * @internal
 * Allows TDEIO classes to display dialog boxes with the correct
 * theme/style even in non-GUI apps like kded and kfmclient
 */
class TDEIO_EXPORT KMessageBoxWrapper : public KMessageBox
{
public:
  static void error(TQWidget *parent, 
                    const TQString &text, 
                    const TQString &caption = TQString::null)
    {
	if (TDEApplication::guiEnabled()) {
     	  kapp->enableStyles();
      	  KMessageBox::error( parent, text, caption );
	} else 
	  kdWarning() << text << endl;
    }

  static void sorry(TQWidget *parent, 
                    const TQString &text,
                    const TQString &caption = TQString::null)
    {
      if (TDEApplication::guiEnabled()) {
	 kapp->enableStyles();
         KMessageBox::sorry( parent, text, caption );
      } else
	kdWarning() << text << endl;
    }
  
};
#endif
