/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kbookmarkmanager.h>
#include <kdebug.h>

#include "kbookmarkdombuilder.h"

KBookmarkDomBuilder::KBookmarkDomBuilder(
   const KBookmarkGroup &bkGroup, KBookmarkManager *manager
) {
   m_manager = manager;
   m_stack.push(bkGroup);
}

KBookmarkDomBuilder::~KBookmarkDomBuilder() {
   m_list.clear();
   m_stack.clear();
}

void KBookmarkDomBuilder::connectImporter(const TQObject *importer) {
   connect(importer, TQ_SIGNAL( newBookmark(const TQString &, const TQCString &, const TQString &) ),
                     TQ_SLOT( newBookmark(const TQString &, const TQCString &, const TQString &) ));
   connect(importer, TQ_SIGNAL( newFolder(const TQString &, bool, const TQString &) ),
                     TQ_SLOT( newFolder(const TQString &, bool, const TQString &) ));
   connect(importer, TQ_SIGNAL( newSeparator() ),
                     TQ_SLOT( newSeparator() ) );
   connect(importer, TQ_SIGNAL( endFolder() ),
                     TQ_SLOT( endFolder() ) );
}

void KBookmarkDomBuilder::newBookmark(
   const TQString &text, const TQCString &url, const TQString &additionalInfo
) {
   KBookmark bk = m_stack.top().addBookmark(
                                    m_manager, text,
                                    KURL( TQString::fromUtf8(url), 106 /*utf8*/ ),
                                    TQString::null, false);
   // store additional info
   bk.internalElement().setAttribute("netscapeinfo", additionalInfo);
}

void KBookmarkDomBuilder::newFolder(
   const TQString & text, bool open, const TQString & additionalInfo
) {
   // we use a qvaluelist so that we keep pointers to valid objects in the stack
   KBookmarkGroup gp = m_stack.top().createNewFolder(m_manager, text, false);
   m_list.append(gp);
   m_stack.push(m_list.last());
   // store additional info
   TQDomElement element = m_list.last().internalElement();
   element.setAttribute("netscapeinfo", additionalInfo);
   element.setAttribute("folded", (open?"no":"yes"));
}

void KBookmarkDomBuilder::newSeparator() {
   m_stack.top().createNewSeparator();
}

void KBookmarkDomBuilder::endFolder() {
   m_stack.pop();
}

#include "kbookmarkdombuilder.moc"
