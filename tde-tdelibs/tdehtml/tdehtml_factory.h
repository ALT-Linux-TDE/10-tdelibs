/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef __tdehtml_factory_h__
#define __tdehtml_factory_h__

#include <tdeparts/factory.h>
#include <tdeparts/historyprovider.h>
#include <tqptrlist.h>
#include <kurl.h>

class TDEInstance;
class TDEAboutData;
class HistoryProvider;
class TDEHTMLSettings;
class TDEHTMLPart;

namespace DOM
{
  class DocumentImpl;
}

class TDE_EXPORT TDEHTMLFactory : public KParts::Factory
{
  TQ_OBJECT
  friend class DOM::DocumentImpl;
  friend class TDEHTMLViewPrivate;
public:
  TDEHTMLFactory( bool clone = false );
  virtual ~TDEHTMLFactory();

  virtual KParts::Part *createPartObject( TQWidget *parentWidget, const char *widgetName, TQObject *parent, const char *name, const char *className, const TQStringList &args );

  static void registerPart( TDEHTMLPart *part );
  static void deregisterPart( TDEHTMLPart *part );

  static TQPtrList<TDEHTMLPart> *partList() { return s_parts; }

  static TDEInstance *instance();

  static TDEHTMLSettings *defaultHTMLSettings();

  // list of visited URLs
  static KParts::HistoryProvider *vLinks() {
    return KParts::HistoryProvider::self();
  }

protected:
  static void ref();
  static void deref();
private:
  static unsigned long s_refcnt;
  static TDEHTMLFactory *s_self;
  static TDEInstance *s_instance;
  static TDEAboutData *s_about;
  static TDEHTMLSettings *s_settings;
  static TQPtrList<TDEHTMLPart> *s_parts;
};

#endif
