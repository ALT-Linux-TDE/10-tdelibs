/* This file is part of the KDE libraries
   Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TDEFILEAUDIOPREVIEW_H
#define TDEFILEAUDIOPREVIEW_H

#include <tqdict.h>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class TQCheckBox;
class TQPushButton;
class TQLabel;

class KFileDialog;
class KFileItem;

/**
 * Audio "preview" widget for the file dialog.
 */
class KFileAudioPreview : public KPreviewWidgetBase
{
    TQ_OBJECT

public:
    KFileAudioPreview(TQWidget *parent = 0, const char *name = 0 );
    ~KFileAudioPreview();

public slots:
    virtual void showPreview(const KURL &url);
    virtual void clearPreview();

private slots:
    void toggleAuto(bool);

private:
    TQDict<void> m_supportedFormats;
    KURL m_currentURL;
    TQCheckBox *m_autoPlay;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileAudioPreviewPrivate;
    KFileAudioPreviewPrivate *d;
};

#endif // TDEFILEAUDIOPREVIEW_H
