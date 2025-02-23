/*
 * This file is part of the KDE project.
 * Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *               
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include "kpreviewwidgetbase.h"
#include <tqstringlist.h>

class KPreviewWidgetBase::KPreviewWidgetBasePrivate
{
public:
    TQStringList supportedMimeTypes;
};

TQPtrDict<KPreviewWidgetBase::KPreviewWidgetBasePrivate> * KPreviewWidgetBase::s_private;

KPreviewWidgetBase::KPreviewWidgetBase( TQWidget *parent, const char *name )
    : TQWidget( parent, name )
{
    if ( !s_private )
        s_private = new TQPtrDict<KPreviewWidgetBasePrivate>();

    s_private->insert( this, new KPreviewWidgetBasePrivate() );
}

KPreviewWidgetBase::~KPreviewWidgetBase()
{
    s_private->remove( this );
    if ( s_private->isEmpty() )
    {
        delete s_private;
        s_private = 0L;
    }
}

void KPreviewWidgetBase::setSupportedMimeTypes( const TQStringList& mimeTypes )
{
    d()->supportedMimeTypes = mimeTypes;
}

TQStringList KPreviewWidgetBase::supportedMimeTypes() const
{
    return d()->supportedMimeTypes;
}

#include "kpreviewwidgetbase.moc"
