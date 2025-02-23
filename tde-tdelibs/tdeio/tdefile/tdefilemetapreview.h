/*
 * This file is part of the KDE project.
 * Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef TDEFILEMETAPREVIEW_H
#define TDEFILEMETAPREVIEW_H

#include <tqdict.h>
#include <tqwidgetstack.h>

#include <kpreviewwidgetbase.h>
#include <kurl.h>

class TDEIO_EXPORT KFileMetaPreview : public KPreviewWidgetBase
{
    TQ_OBJECT

public:
    KFileMetaPreview(TQWidget *parent, const char *name = 0);
    ~KFileMetaPreview();

    virtual void addPreviewProvider( const TQString& mimeType,
                                     KPreviewWidgetBase *provider );
    virtual void clearPreviewProviders();

public slots:
    virtual void showPreview(const KURL &url);
    virtual void clearPreview();

protected:
    virtual KPreviewWidgetBase *previewProviderFor( const TQString& mimeType );

protected:
    virtual void virtual_hook( int id, void* data );
    
private:
    void initPreviewProviders();

    TQWidgetStack *m_stack;
    TQDict<KPreviewWidgetBase> m_previewProviders;
    bool haveAudioPreview;

    // may return 0L
    static KPreviewWidgetBase * createAudioPreview( TQWidget *parent );
    static bool s_tryAudioPreview;

private:
    class KFileMetaPreviewPrivate;
    KFileMetaPreviewPrivate *d;
};

#endif // TDEFILEMETAPREVIEW_H
