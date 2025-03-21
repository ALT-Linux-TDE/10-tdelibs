/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCLIPBOARD_H
#define KCLIPBOARD_H

#include <tqclipboard.h>
#include <tqmime.h>
#include <tqobject.h>
#include <tqstrlist.h>
#include "tdelibs_export.h"

/**
 * This class is only for internal use.
 *
 * @short Allowing to automatically synchronize the X11 Clipboard and Selection buffers.
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @since 3.1
 * @internal
 */
class TDECORE_EXPORT TDEClipboardSynchronizer : public TQObject
{
    TQ_OBJECT

public:
    /** Systray widget for manipulating the clipboard. */
    friend class KlipperWidget;
    friend class TDEApplication;

    /**
     * Returns the TDEClipboardSynchronizer singleton object.
     * @return the TDEClipboardSynchronizer singleton object.
     */
    static TDEClipboardSynchronizer *self();

    /**
     * Configures TDEClipboardSynchronizer to synchronize the Selection to Clipboard whenever
     * it changes.
     *
     * Default is false.
     * @see isSynchronizing
     */
    static void setSynchronizing( bool sync );

    /**
     * Checks whether Clipboard and Selection will be synchronized upon changes.
     * @returns whether Clipboard and Selection will be synchronized upon
     * changes.
     * @see setSynchronizing
     */
    static bool isSynchronizing()
    {
        return s_sync;
    }

    /**
     * Configures TDEClipboardSynchronizer to copy the Clipboard buffer to the Selection
     * buffer whenever the Clipboard changes.
     *
     *
     * @param enable true to enable implicit selection, false otherwise.
     * Default is true.
     * @see selectionSetting
     */
    static void setReverseSynchronizing( bool enable );

    /**
     * Checks whether the  Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @returns whether the Clipboard buffer will be copied to the Selection
     * buffer upon changes.
     * @see setSelectionSetting
     */
    static bool isReverseSynchronizing()
    {
        return s_reverse_sync;
    }


protected:
    ~TDEClipboardSynchronizer();

private slots:
    void slotSelectionChanged();
    void slotClipboardChanged();

private:
    TDEClipboardSynchronizer( TQObject *parent = 0, const char *name = 0L );
    void setupSignals();

    static void setClipboard( TQMimeSource* data, TQClipboard::Mode mode );

    static TDEClipboardSynchronizer *s_self;
    static bool s_sync;
    static bool s_reverse_sync;
    static bool s_blocked;

    class MimeSource;

private:
    // needed by klipper
    enum Configuration { Synchronize = 1 };
    // called by TDEApplication upon kipc message, invoked by klipper
    static void newConfiguration( int config );

};

#endif // KCLIPBOARD_H
