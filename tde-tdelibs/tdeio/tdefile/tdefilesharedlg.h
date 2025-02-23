/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TDEFILESHAREDLG_H
#define TDEFILESHAREDLG_H

#include <kpropertiesdialog.h>
class TQVBoxLayout;
class TQRadioButton;
class TQPushButton;

/**
 * This plugin provides a page to KPropsDlg, showing the "file sharing" options
 * @author David Faure <david@mandrakesoft.com>
 * @since 3.1
 */
class TDEIO_EXPORT KFileSharePropsPlugin : public KPropsDlgPlugin
{
    TQ_OBJECT
public:
    KFileSharePropsPlugin( KPropertiesDialog *_props );
    virtual ~KFileSharePropsPlugin();

    /**
     * Apply all changes to the file.
     * This function is called when the user presses 'Ok'. The last plugin inserted
     * is called first.
     */
    virtual void applyChanges();

    static bool supports( const KFileItemList& items );

    TQWidget* page() const;

protected slots:
    void slotConfigureFileSharing();
    void slotConfigureFileSharingDone();

private:
    void init();
    bool setShared( const TQString&path, bool shared );
    bool SuSEsetShared( const TQString&path, bool shared, bool readonly );

    TQWidget *m_widget;
    TQRadioButton *m_rbShare;
    TQRadioButton *m_rbSharerw;
    TQRadioButton *m_rbUnShare;
    //TQLineEdit    *m_leSmbShareName;
    TQPushButton *m_pbConfig;
    class Private;
    Private *d;
};

#endif
