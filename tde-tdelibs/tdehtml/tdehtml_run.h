/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
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
#ifndef __tdehtml_run_h__
#define __tdehtml_run_h__

#include <tdeparts/browserrun.h>
#include <kurl.h>
#include <kservice.h>
#include <tdeparts/browserextension.h>

class TDEHTMLPart;

namespace tdehtml
{
  class ChildFrame;
}

class TDEHTMLRun : public KParts::BrowserRun
{
  TQ_OBJECT
public:
  TDEHTMLRun( TDEHTMLPart *part, tdehtml::ChildFrame *child, const KURL &url,
            const KParts::URLArgs &args, bool hideErrorDialog );

  virtual void foundMimeType( const TQString &mimetype );

  //TDEHTMLPart *htmlPart() const;

protected:
  virtual void handleError( TDEIO::Job * job );

  virtual void save( const KURL & url, const TQString & suggestedFilename );
  bool askSave( const KURL & url, KService::Ptr offer, const TQString & mimeType, const TQString & suggestedFilename );

private:
  tdehtml::ChildFrame *m_child;
};

#endif
