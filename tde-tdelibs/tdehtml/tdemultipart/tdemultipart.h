/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <david@mandrakesoft.com>

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

#ifndef __tdemultipart_h__
#define __tdemultipart_h__

#include <httpfilter/httpfilter.h>

#include <tdeparts/part.h>
#include <tdeparts/factory.h>
#include <tdeparts/browserextension.h>
#include <tdeaboutdata.h>
#include <tqdatetime.h>

class TDEHTMLPart;
class TDEInstance;
class KTempFile;
class KLineParser;

/**
 * http://www.netscape.com/assist/net_sites/pushpull.html
 */
class KMultiPart : public KParts::ReadOnlyPart
{
    TQ_OBJECT
public:
    KMultiPart( TQWidget *parentWidget, const char *widgetName,
                TQObject *parent, const char *name, const TQStringList& );
    virtual ~KMultiPart();

    virtual bool openFile() { return false; }
    virtual bool openURL( const KURL &url );

    virtual bool closeURL();

    static TDEAboutData* createAboutData();

protected:
    virtual void guiActivateEvent( KParts::GUIActivateEvent *e );
    void setPart( const TQString& mimeType );

    void startOfData();
    void sendData( const TQByteArray& line );
    void endOfData();

private slots:
    void reallySendData( const TQByteArray& line );
    //void slotPopupMenu( KXMLGUIClient *cl, const TQPoint &pos, const KURL &u, const TQString &mime, mode_t mode );
    void slotJobFinished( TDEIO::Job *job );
    void slotData( TDEIO::Job *, const TQByteArray & );
    //void updateWindowCaption();

    void slotPartCompleted();

    void startHeader();

    void slotProgressInfo();

private:
    KParts::BrowserExtension* m_extension;
    TQGuardedPtr<KParts::ReadOnlyPart> m_part;
    bool m_isHTMLPart;
    bool m_partIsLoading;
    TDEIO::Job* m_job;
    TQCString m_boundary;
    int m_boundaryLength;
    TQString m_mimeType; // the one handled by m_part - store the kservice instead?
    TQString m_nextMimeType; // while parsing headers
    KTempFile* m_tempFile;
    KLineParser* m_lineParser;
    bool m_bParsingHeader;
    bool m_bGotAnyHeader;
    bool m_gzip;
    HTTPFilterBase *m_filter;
    // Speed measurements
    long m_totalNumberOfFrames;
    long m_numberOfFrames;
    long m_numberOfFramesSkipped;
    TQTime m_qtime;
    TQTimer* m_timer;
};

#if 0
class KMultiPartBrowserExtension : public KParts::BrowserExtension
{
    //TQ_OBJECT
public:
    KMultiPartBrowserExtension( KMultiPart *parent, const char *name = 0 );

    virtual int xOffset();
    virtual int yOffset();

//protected slots:
    void print();
    void reparseConfiguration();

private:
    KMultiPart *m_imgPart;
};
#endif

#endif
