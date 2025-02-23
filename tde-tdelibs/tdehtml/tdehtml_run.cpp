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
#include "tdehtmlpart_p.h"
#include "tdehtml_run.h"
#include <tdeio/job.h>
#include <kdebug.h>
#include <tdelocale.h>
#include "tdehtml_ext.h"
#include <tqwidget.h>

TDEHTMLRun::TDEHTMLRun( TDEHTMLPart *part, tdehtml::ChildFrame *child, const KURL &url,
                    const KParts::URLArgs &args, bool hideErrorDialog )
    : KParts::BrowserRun( url, args, part, part->widget() ? part->widget()->topLevelWidget() : 0,
                          false, false, hideErrorDialog ),
  m_child( child )
{
    // Don't use an external browser for parts of a webpage we are rendering. (iframes at least are one example)
    setEnableExternalBrowser(false);

    // get the wheel to start spinning
    part->started(0L);
}

//TDEHTMLPart *TDEHTMLRun::htmlPart() const
//{ return static_cast<TDEHTMLPart *>(m_part); }

void TDEHTMLRun::foundMimeType( const TQString &_type )
{
    Q_ASSERT(!m_bFinished);
    TQString mimeType = _type; // this ref comes from the job, we lose it when using TDEIO again
    if ( static_cast<TDEHTMLPart *>(m_part)->processObjectRequest( m_child, m_strURL, mimeType ) )
        m_bFinished = true;
    else {
        if ( m_bFinished ) // abort was called (this happens with the activex fallback for instance)
            return;
        // Couldn't embed -> call BrowserRun::handleNonEmbeddable()
        KParts::BrowserRun::NonEmbeddableResult res = handleNonEmbeddable( mimeType );
        if ( res == KParts::BrowserRun::Delayed )
            return;
        m_bFinished = ( res == KParts::BrowserRun::Handled );
        if ( m_bFinished ) { // saved or canceled -> flag completed
            m_child->m_bCompleted = true;
            static_cast<TDEHTMLPart *>(m_part)->checkCompleted();
        }
    }

    if ( m_bFinished )
    {
        m_timer.start( 0, true );
        return;
    }

    //kdDebug(6050) << "TDEHTMLRun::foundMimeType " << _type << " couldn't open" << endl;
    KRun::foundMimeType( mimeType );

    // "open" is finished -> flag completed
    m_child->m_bCompleted = true;
    static_cast<TDEHTMLPart *>(m_part)->checkCompleted();
}

void TDEHTMLRun::save( const KURL & url, const TQString & suggestedFilename )
{
    TDEHTMLPopupGUIClient::saveURL( m_part->widget(), i18n( "Save As" ), url, m_args.metaData(), TQString::null, 0, suggestedFilename );
}

// KDE4: remove
void TDEHTMLRun::handleError( TDEIO::Job *job )
{
    KParts::BrowserRun::handleError( job );
}

#include "tdehtml_run.moc"
