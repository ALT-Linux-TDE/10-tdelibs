/* This file is part of the KDE project
 *
 * Copyright (C) 2002 Stephan Kulow <coolo@kde.org>
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

#include "tdehtml_iface.h"
#include "tdehtml_part.h"
#include "tdehtmlview.h"
#include "tdehtml_ext.h"
#include <tdeio/global.h>
#include <tqapplication.h>
#include <tqvariant.h>

TDEHTMLPartIface::TDEHTMLPartIface( TDEHTMLPart *_part )
    : DCOPObject( _part->dcopObjectId() ), part(_part)
{
}

TDEHTMLPartIface::~TDEHTMLPartIface()
{
}

KURL TDEHTMLPartIface::url() const
{
    return part->url();
}

void TDEHTMLPartIface::setJScriptEnabled( bool enable )
{
    part->setJScriptEnabled(enable);
}

bool TDEHTMLPartIface::jScriptEnabled() const
{
    return part->jScriptEnabled();
}

bool TDEHTMLPartIface::closeURL()
{
    return part->closeURL();
}

bool TDEHTMLPartIface::metaRefreshEnabled() const
{
    return part->metaRefreshEnabled();
}

void TDEHTMLPartIface::setDNDEnabled( bool b )
{
    part->setDNDEnabled(b);
}

bool TDEHTMLPartIface::dndEnabled() const
{
    return part->dndEnabled();
}

void TDEHTMLPartIface::setJavaEnabled( bool enable )
{
    part->setJavaEnabled( enable );
}

bool TDEHTMLPartIface::javaEnabled() const
{
    return part->javaEnabled();
}

void TDEHTMLPartIface::setPluginsEnabled( bool enable )
{
    part->setPluginsEnabled( enable );
}

bool TDEHTMLPartIface::pluginsEnabled() const
{
    return part->pluginsEnabled();
}

void TDEHTMLPartIface::setAutoloadImages( bool enable )
{
    part->setAutoloadImages( enable );
}

bool TDEHTMLPartIface::autoloadImages() const
{
    return part->autoloadImages();
}

void TDEHTMLPartIface::setOnlyLocalReferences(bool enable)
{
    part->setOnlyLocalReferences(enable);
}

void TDEHTMLPartIface::setMetaRefreshEnabled( bool enable )
{
    part->setMetaRefreshEnabled(enable);
}

bool TDEHTMLPartIface::onlyLocalReferences() const
{
    return part->onlyLocalReferences();
}

bool TDEHTMLPartIface::setEncoding( const TQString &name )
{
    return part->setEncoding(name);
}

TQString TDEHTMLPartIface::encoding() const
{
    return part->encoding();
}

void TDEHTMLPartIface::setFixedFont( const TQString &name )
{
    part->setFixedFont(name);

}

bool TDEHTMLPartIface::gotoAnchor( const TQString &name )
{
    return part->gotoAnchor(name);
}

bool TDEHTMLPartIface::nextAnchor()
{
    return part->nextAnchor();
}

bool TDEHTMLPartIface::prevAnchor()
{
    return part->prevAnchor();
}

void TDEHTMLPartIface::activateNode()
{
    KParts::ReadOnlyPart* p = part->currentFrame();
    if ( p && p->widget() ) {
        TQKeyEvent ev( TQKeyEvent::KeyPress, TQt::Key_Return, '\n', 0, "\n" );
        TQApplication::sendEvent( p->widget(), &ev );
    }
}

void TDEHTMLPartIface::selectAll()
{
    part->selectAll();
}

TQString TDEHTMLPartIface::lastModified() const
{
    return part->lastModified();
}

void TDEHTMLPartIface::debugRenderTree()
{
    part->slotDebugRenderTree();
}

void TDEHTMLPartIface::debugDOMTree()
{
    part->slotDebugDOMTree();
}

void TDEHTMLPartIface::stopAnimations()
{
    part->slotStopAnimations();
}

void TDEHTMLPartIface::viewDocumentSource()
{
    part->slotViewDocumentSource();
}

void TDEHTMLPartIface::saveBackground(const TQString &destination)
{
    KURL back = part->backgroundURL();
    if (back.isEmpty())
        return;

    TDEIO::MetaData metaData;
    metaData["referrer"] = part->referrer();
    TDEHTMLPopupGUIClient::saveURL( back, KURL( destination ), metaData );
}

void TDEHTMLPartIface::saveDocument(const TQString &destination)
{
    KURL srcURL( part->url() );

    if ( srcURL.fileName(false).isEmpty() )
        srcURL.setFileName( "index.html" );

    TDEIO::MetaData metaData;
    // Referrer unknown?
    TDEHTMLPopupGUIClient::saveURL( srcURL, KURL( destination ), metaData, part->cacheId() );
}

void TDEHTMLPartIface::setUserStyleSheet(const TQString &styleSheet)
{
    part->setUserStyleSheet(styleSheet);
}

TQString TDEHTMLPartIface::selectedText() const
{
    return part->selectedText();
}

void TDEHTMLPartIface::viewFrameSource()
{
    part->slotViewFrameSource();
}

TQString TDEHTMLPartIface::evalJS(const TQString &script)
{
    return part->executeScript(DOM::Node(), script).toString();
}

void TDEHTMLPartIface::print( bool quick ) {
    part->view()->print( quick );
}
