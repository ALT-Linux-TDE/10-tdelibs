/*
   Copyright (C) 2000, 2001, 2002 Dawit Alemayehu <adawit@kde.org>

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

#ifndef __RSYNC_CONFIG_DIALOG_H
#define __RSYNC_CONFIG_DIALOG_H

#include <tqmap.h>
#include <tqstringlist.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>

#include <kurl.h>
#include <kprocess.h>
#include <tdefileitem.h>
#include <klibloader.h>
#include <kdialogbase.h>

// NOTE: If ANY of these functions are not utilized in the C file,
// and in a manner identical to these declarations, the plugin will
// mysteriously fail when launched with kshell but work fine under BASH

class RsyncConfigDialog : public KDialogBase
{
    TQ_OBJECT
  

    public:
        RsyncConfigDialog(TQWidget* parent = 0, const char* name = 0,
                        const TQString& caption = TQString(),
                        const TQString& text = TQString(),
                        const TQString& localfolder = TQString(),
                        const TQString& remotefolder = TQString(),
                        int syncmode = 1, bool modal = false);

         /**
         * Returns the TQLineEdit used in this dialog.
         * To set the number of lines or other text box related
         * settings, access the KTextEdit object directly via this method.
         */
        TQLineEdit* lineEdit();

        /**
         * Returns the TQLineEdit used in this dialog.
         * To set the number of lines or other text box related
         * settings, access the KTextEdit object directly via this method.
         */
        const TQLineEdit* lineEdit() const;

        /**
         * Returns index of selected synchronization mode
         * 1: Upload
         * 2: Download
         * 3: Bidirectional
         */
        int getSyncMode();

        /**
         * Returns the selected autosync flags
         * Bit 0: Sync on logout
         */
        int getAutoSyncFlags();

        /**
         * Sets the selected autosync flags
         *
         * @see getAutoSyncFlags()
         */
        void setAutoSyncFlags(int flags);

    private:
        bool       mAutoClose;
        bool       mAutoReset;
        bool       mCancelled;
        bool       mAllowCancel;
        bool       mAllowTextEdit;
        bool       mShown;
        bool       mSyncAutoLogout;
        TQString    mCancelText;
        TQLabel*    mLabel;
        KProgress* mProgressBar;
        KTextEdit* mTextBox;
        TQTimer*    mShowTimer;
        TQLineEdit* m_rsync_txt;
        TQCheckBox* m_sync_auto_logout_cb;
        TQRadioButton *rsync_rb1;
        TQRadioButton *rsync_rb2;
        TQRadioButton *rsync_rb3;
};
#endif
