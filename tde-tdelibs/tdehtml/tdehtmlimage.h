/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

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

#ifndef __tdehtmlimage_h__
#define __tdehtmlimage_h__

#include "tdehtml_part.h"
#include <tdeparts/factory.h>
#include <tdeparts/browserextension.h>

#include "misc/loader_client.h"

class TDEHTMLPart;
class TDEInstance;

namespace tdehtml
{
    class CachedImage;
}

/**
 * @internal
 */
class TDEHTMLImageFactory : public KParts::Factory
{
    TQ_OBJECT
public:
    TDEHTMLImageFactory();
    virtual ~TDEHTMLImageFactory();

    virtual KParts::Part *createPartObject( TQWidget *parentWidget, const char *widgetName,
                                            TQObject *parent, const char *name,
                                            const char *className, const TQStringList &args );

    static TDEInstance *instance() { return s_instance; }

private:
    static TDEInstance *s_instance;
};

/**
 * @internal
 */
class TDEHTMLImage : public KParts::ReadOnlyPart, public tdehtml::CachedObjectClient
{
    TQ_OBJECT
public:
    TDEHTMLImage( TQWidget *parentWidget, const char *widgetName,
                TQObject *parent, const char *name, TDEHTMLPart::GUIProfile prof );
    virtual ~TDEHTMLImage();

    virtual bool openFile() { return true; } // grmbl, should be non-pure in part.h, IMHO

    virtual bool openURL( const KURL &url );

    virtual bool closeURL();

    TDEHTMLPart *doc() const { return m_tdehtml; }

    virtual void notifyFinished( tdehtml::CachedObject *o );

protected:
    virtual void guiActivateEvent( KParts::GUIActivateEvent *e );
    virtual bool eventFilter( TQObject *filterTarget, TQEvent *e );

private slots:
    void restoreScrollPosition();
//    void slotImageJobFinished( TDEIO::Job *job );

//    void updateWindowCaption();

private:
    void disposeImage();

    TQGuardedPtr<TDEHTMLPart> m_tdehtml;
    KParts::BrowserExtension *m_ext;
    TQString m_mimeType;
    tdehtml::CachedImage *m_image;
    int m_xOffset, m_yOffset;
};

/**
 * @internal
 */
class TDEHTMLImageBrowserExtension : public KParts::BrowserExtension
{
    TQ_OBJECT
public:
    TDEHTMLImageBrowserExtension( TDEHTMLImage *parent, const char *name = 0 );

    virtual int xOffset();
    virtual int yOffset();

protected slots:
    void print();
    void reparseConfiguration();
    void disableScrolling();

private:
    TDEHTMLImage *m_imgPart;
};

#endif
